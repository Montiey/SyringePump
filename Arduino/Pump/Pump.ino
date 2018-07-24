/// Copyright 2018 Jason Harriot
/// Units: Seconds, uL / sec

#include "AccelStepper.h"
#include "Montiey_Util.h"
#include <SPI.h>
#include <SD.h>

#define LED LED_BUILTIN

File commandstxt;

#define BUTTON 5
#define STEP 4
#define DIR 2
#define UL_PER_STEP 0.01973325157   // Calculated value for 1/32 microstepping, 0.9deg stepper & 0.8mm rod
String sample;

AccelStepper s(1, STEP, DIR); //1 = "driver mode" (operate with pulse and direction pins)
HandyTimer recalculationInterval(250); //Only set the stepper speed so often. AccelStepper no likey < 50ms
int commandIndex = 0;   //Current line of text
unsigned long offsetTime = 0;   //Time at which the routine was started (with BUTTON)

////////

void(* reset)(void) = 0;    //I don't know how this works

String getLine(String s, int line) {
    if (line < 0) {
        return "";
    } else if (line == 0) {
        String line = s.substring(0, s.indexOf('\n'));
        return line;
    } else {
        return getLine(s.substring(s.indexOf('\n') + 1), line - 1); //recursive call
    }
}

void endGame() {    //Stops everything until the button is pressed (i.e. after commands complete, or after an error to retry)
    Serial.println("END");
    while (1) {
        if (!digitalRead(BUTTON)) {
            reset();
        }
    }
}

void doOpenLoopStuff() { //everything that needs to be done as often as possible
    s.runSpeed(); //Check if AccelStepper should send a step to the stepper based on the set speed
}

void setQ(float q) {
    float stepSpeed = (1.0 / UL_PER_STEP) * q;
    Serial.print("Set q: ");
    Serial.print(q);
    Serial.print(" Set speed: ");
    Serial.println(stepSpeed);
    s.setSpeed(stepSpeed);
}

void setup() {
    Serial.begin(250000);
    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    digitalWrite(BUTTON, HIGH); //internal pullup (trigger on low)

    if (!SD.begin()) {
        Serial.println("SD card not present");
        endGame();
    }
    commandstxt = SD.open("commands.txt");
    if (!commandstxt) {
        Serial.println("commands.txt not found");
        endGame();
    }
    while (commandstxt.available()) {
        sample += (char) commandstxt.read();
    }
    if (sample.lastIndexOf('\n') > 0) {  //make sure that the string ends with a newline, otherwise the last command repeats
        Serial.println("newline added");
        sample += "\n";
    }

    Serial.println("Sample:\n" + sample);

    s.setMaxSpeed(1000000); //Something to do with acceleration, which we don't care about. Just use a very high number.
    s.setSpeed(0);  //Don't move anything do start with

//    while (digitalRead(BUTTON)) {   //wait for the BUTTON to begin the routine
//        offsetTime = millis();
//        doOpenLoopStuff();
//    }
}

void loop() {
    String cmd = getLine(sample, commandIndex);

    float t;
    float ta;
    float q;
    float qa;
    float tb;
    float qb;

    String a = "This event is being queued: ";
    if (cmd.indexOf("qa") > -1) {   //if command is for gradient
        Serial.println("============================ This is a gradient command");
        ta = cmd.substring(cmd.indexOf("ta") + 2).toFloat(); //toFloat ignores whitespace
        qa = cmd.substring(cmd.indexOf("qa") + 2).toFloat();
        tb = cmd.substring(cmd.indexOf("tb") + 2).toFloat();
        qb = cmd.substring(cmd.indexOf("qb") + 2).toFloat();
        Serial.println(a + commandIndex); Serial.println(ta); Serial.println(q); Serial.println(qa); Serial.println(tb); Serial.println(qb);
        Serial.println("Command: " + cmd);
        if (t == 0 && commandIndex != 0) {
            Serial.println("This command has stopped the routine: " + cmd);
            endGame(); //Cheap way of figuring out if String.toFloat failed
        }

        while (1) {
            if (millis() - offsetTime >= ta * 1000) { //if the queued event is up or has passed
                break;
            }
            doOpenLoopStuff();  //While we wait, do all the things.
        }

        unsigned long startTime = offsetTime + (ta * 1000); //start of the gradient where it SHOULD be, not where control is
        while (1) { //Loop for the duration of the gradient command

            if (recalculationInterval.trigger()) {
                float newQ = (float) map((millis() - startTime), 0.0, (tb - ta) * 1000.0, qa, qb);  // q at any point across the gradient using TIME from [0, target time] to [start speed, end speed]
                setQ(newQ);
            }

            if ((millis() - startTime) > (tb - ta) * 1000) { //if the gradient is complete
                //Don't neccessarily stop pumping, continue at the set q. (unless this was the last command, then prog will end immediately.)
                break;
            }
            doOpenLoopStuff();  //While we wait, do all the things.
        }
    } else {
        Serial.println("============================ This is an instantaneous command");
        t = cmd.substring(cmd.indexOf('t') + 1).toFloat();
        q = cmd.substring(cmd.indexOf('q') + 1).toFloat();
        Serial.println(a + commandIndex); Serial.println(t); Serial.println(q);
        Serial.println("Command: " + cmd);
        if (t == 0 && commandIndex != 0) {
            Serial.println("This command has stopped the routine: " + cmd);
            endGame(); //Cheap way of figuring out if String.toFloat failed
        }

        while (1) {
            if (millis() - offsetTime >= t * 1000) { //if the queued event is up or has passed
                break;
            }
            doOpenLoopStuff();    //While we wait, do all the things.
        }
        setQ(q);
    }

    commandIndex++;    //target the next line and never look back
}
