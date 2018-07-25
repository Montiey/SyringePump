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
unsigned long offsetTime = 0;   //Time at which the routine was started (with button)

////////

void(* reset)(void) = 0;    //I don't know how this works

String getLine(String s, int line) {
    Serial.print("[getLine()] Getting from line: ");
    Serial.println(line);
    Serial.print("[getLine()] String: " + s);
    
    if (line < 0) {
        return "";  //return nothing so getFloat can return 0
    } else if (line == 0) {
        Serial.print("[getLine()] Result: " + s.substring(0, s.indexOf('\n')));
        return s.substring(0, s.indexOf('\n'));
        
    } else {
        Serial.println("Recursion neccesary. I will now return: ");
        Serial.println(getLine(s.substring(s.indexOf('\n') + 1), line - 1));
        return getLine(s.substring(s.indexOf('\n') + 1), line - 1); //recursive call
    }
}

void loadSD(){
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
    commandstxt.close();
    if (sample.lastIndexOf('\n') > 0) {  //make sure that the string ends with a newline, otherwise the last command repeats
        Serial.println("Newline added because there wasn't one at the end");
        sample += "\n";
    }

    Serial.print("Finalized sample:\n" + sample);
}

void endGame() {    //Stops everything until the button is pressed (i.e. after commands complete, or after an error to retry)
    delay(1000);
    Serial.println("END");
    while(!digitalRead(BUTTON));    //do a little debouncing
    delay(1000);
    while (digitalRead(BUTTON));
    reset();
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
    Serial.begin(115200);
    Serial.println("======== Begin ========");
    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    digitalWrite(BUTTON, HIGH); //internal pullup (trigger on low)

    loadSD();
    

    s.setMaxSpeed(1000000); //Something to do with acceleration, which we don't care about. Just use a very high number.
    s.setSpeed(0);  //Don't move anything do start with

    while (digitalRead(BUTTON)) {   //wait for the button to begin the routine
        doOpenLoopStuff();
    }
    offsetTime = millis();
}

void loop() {
    Serial.println("Loop is now running. Getting next command: ");
    String cmd = getLine(sample, commandIndex);

    
    if (cmd.indexOf("qa") > -1) {   //if command is for gradient
    float ta;
    float qa;
    float tb;
    float qb;
        Serial.println("============================ Begin gradient command");
        Serial.println("Command: " + cmd);
        Serial.println("Sample at this point:\n" + sample);


        //Get data
        ta = cmd.substring(cmd.indexOf("ta") + 2).toFloat(); //toFloat ignores whitespace
        qa = cmd.substring(cmd.indexOf("qa") + 2).toFloat();
        tb = cmd.substring(cmd.indexOf("tb") + 2).toFloat();
        qb = cmd.substring(cmd.indexOf("qb") + 2).toFloat();
        Serial.print("Command index: ");
        Serial.println(commandIndex);
        Serial.println(ta);
        Serial.println(qa);
        Serial.println(tb);
        Serial.println(qb);
        if (ta == 0 && commandIndex != 0) { //Cheap way of figuring out if String.toFloat failed
            Serial.println("This command has stopped the routine.");
            endGame();
        }

        while (1) {
            if (millis() - offsetTime >= ta * 1000) { //if the queued event is up or has passed
                break;
            }
            doOpenLoopStuff();  //While we wait, do all the things.
        }

        unsigned long startTime = offsetTime + (ta * 1000); //start of the gradient where it SHOULD be
        while (1) { //Loop for the duration of the gradient command

            if (recalculationInterval.trigger()) {
                float newQ = (float) map((millis() - startTime), 0.0, (tb - ta) * 1000.0, qa, qb);
                setQ(newQ);
            }

            if ((millis() - startTime) > (tb - ta) * 1000) { //if the gradient is complete
                break;
            }
            doOpenLoopStuff();  //While we wait, do all the things.
        }
    } else {
        Serial.println("============================ Begin normal command");
        Serial.println("Command: " + cmd);
        Serial.println("Sample at this point:\n" + sample);
        float t;
        float q;

        //Get data
        t = cmd.substring(cmd.indexOf('t') + 1).toFloat();
        q = cmd.substring(cmd.indexOf('q') + 1).toFloat();
        Serial.print("Command index: "); Serial.println(commandIndex); Serial.println(t); Serial.println(q);
        if (t == 0 && commandIndex != 0) { //Cheap way of figuring out if String.toFloat failed
            Serial.println("This command has stopped the routine.");
            endGame();
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
