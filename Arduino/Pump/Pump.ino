/// Copyright 2018 Jason Harriot
/// Units: Seconds, uL (mm3) / sec
/// General notes: Serial prints take alot of time, so plan on using the pump without them.

//////// vvv USER CONFIGURATION vvv ////////

////////////////////////
/// Removes ALL serial commands (for production)
//#define NOSERIAL//////
////////////////////////

#ifndef NOSERIAL
//////////////////////
/// Enables spammy debugging prints (for testing) (if not already disabled above)
#define DEBUGSERIAL//////
//////////////////////
#endif


//////////////////////
#define ID 18.5    //Inner diameter of the syringe
#define PITCH 0.8  //Pitch of the threaded rod
#define STEPS 400 //Steps per revolution of the motor
#define USTEP_RATE 32   //This sets digital pins to control microstepping (1, 2, 4, 8, 16, or 32)
#define TUNE 1  //Multiplier applied to step speed (2 = twice the fluid)


//////////////////////
const float UL_PER_STEP = PITCH * (360.0 / (STEPS * USTEP_RATE) ) * (PI * (float)(pow(ID/2, 2)));   //mm3 /

//////////////////////



////////

#include "AccelStepper.h"
#include "Montiey_Util.h"
#include <SPI.h>
#include <SD.h>

File dataFile;

const char flagT = 't'; //defined here because strstr and strchr don't like literals
const char flagQ = 'q';
const char * flagTA = "ta";
const char * flagQA = "qa";
const char * flagTB = "tb";
const char * flagQB = "qb";

#define MAX_LINE_BYTES 64
#define BUTTONSEL A3
#define BUTTONF A2
#define BUTTONR A1
#define LEDR 4
#define LEDY 3
#define LEDG 2
#define STEP 5
#define DIR 4
#define SDCS 10 //SPI CS
#define MODE0 8
#define MODE1 7
#define MODE2 6

char * dataLine = (char *) calloc(MAX_LINE_BYTES, 1);    //A single command line may contain 64 characters. Increase if more memory is availible.

AccelStepper s(1, STEP, DIR); //1 = "driver mode" (operate with pulse and direction pins)
//MicroTimer stepTimer(0);
HandyTimer recalculationInterval(125); //How often to perform (lengthy) calculations and set the updated speed
//bool stepState;
int commandIndex = 0;   //Current line of text
unsigned long offsetTime = 0;   //Time at which the routine was started (with button)

////////

void(* reset)(void) = 0;    //I don't know how this works

void getLine() {
    bool addNewline = false;
    if(!dataFile.available()) {
#ifdef DEBUGSERIAL
        Serial.println("There is no remaining text in the file. Appending a possibly redundant newline.");
#endif
        addNewline = true;
        endGame();
    }

    char c = ' ';    //some irrelevant starting value
    byte i = 0; //counter

    char x;
    for(x = 0; x < MAX_LINE_BYTES; x++) {
        dataLine[x] = 0x00;
    }
    if(addNewline) {
        dataLine[x+1] = '\n';
    }

    while (dataFile.available() && c != '\n' && i < MAX_LINE_BYTES) { //while there is data to be read
        c = dataFile.read(); //read in the data
#ifdef DEBUGSERIAL
        Serial.print("New character loaded: ");
        Serial.println(c);
#endif
        dataLine[i] = c;
#ifdef DEBUGSERIAL
        Serial.print("Line so far: ");
        Serial.println(dataLine);
#endif
        i++;
    }
#ifdef DEBUGSERIAL
    Serial.print("[getLine()] Line content: ");
    Serial.println(dataLine);
#endif
}

void endGame() {    //Stops everything until the button is pressed (i.e. after commands complete, or after an error to retry)
//    stepTimer.disable = true;   //Will get re-enabled the first time setQ() is validly called
#ifndef NOSERIAL
    Serial.println("END - Waiting for button");
#endif
    while (digitalRead(BUTTONSEL));
    digitalWrite(LEDR, LOW);
    reset();
}

void doOpenLoopStuff() { //everything that needs to be done as often as possible
    s.runSpeed();
}

void setQ(float q) {
    float stepSpeed = UL_PER_STEP * q;
#ifdef DEBUGSERIAL
    Serial.print("Set q: ");
    Serial.print(q);
    Serial.print(" Set speed: ");
    Serial.println(stepSpeed);
#endif
    s.setSpeed(stepSpeed * TUNE);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setMode(bool p0, bool p1, bool p2){
    digitalWrite(MODE0, p0);
    digitalWrite(MODE1, p1);
    digitalWrite(MODE2, p2);
}


////////////////
////////////////



void setup() {
#ifndef NOSERIAL
    Serial.begin(115200);
    Serial.print("µL per step: ");
    Serial.println(UL_PER_STEP);
#endif
#ifdef DEBUGSERIAL
    Serial.println("======== Begin ========");
#endif
    pinMode(MODE0, OUTPUT);
    pinMode(MODE1, OUTPUT);
    pinMode(MODE2, OUTPUT);

    switch(USTEP_RATE) {
    case 1:
        setMode(0, 0, 0);
        break;
    case 2:
        setMode(1, 0, 0);
        break;
    case 4:
        setMode(0, 1, 0);
        break;
    case 8:
        setMode(1, 1, 0);
        break;
    case 16:
        setMode(0, 0, 1);
        break;
    case 32:
        setMode(1, 1, 1);
        break;
    }

    pinMode(SDCS, OUTPUT);

    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDY, OUTPUT);

    pinMode(BUTTONSEL, INPUT);
    pinMode(BUTTONF, INPUT);
    pinMode(BUTTONR, INPUT);

    digitalWrite(BUTTONSEL, HIGH); //internal pullup
    digitalWrite(BUTTONF, HIGH);
    digitalWrite(BUTTONR, HIGH);


    if (!SD.begin(SDCS)) {
#ifndef NOSERIAL
        Serial.println("SD card not present");
        digitalWrite(LEDR, HIGH);
#endif
        endGame();
    }
    dataFile = SD.open("commands.txt");
    if (!dataFile) {
#ifndef NOSERIAL
        Serial.println("commands.txt not found");
#endif
        endGame();
    }

    s.setMaxSpeed(1000000); //Something to do with acceleration, which we don't care about. Just use a very high number that we won't hit
    s.setSpeed(0);  //Don't move anything do start with

    while (digitalRead(BUTTONSEL)) {   //wait for the button to begin the routine
        //Nothing happens
    }
    offsetTime = millis();
}

void loop() {
#ifdef DEBUGSERIAL
    Serial.println("================= Loop is now running. Getting next command: ");
#endif
    getLine(); //get the next line

    if (strstr(dataLine, flagQA)) {   //if command is for gradient (if qa flag exists)
        float ta;
        float qa;
        float tb;
        float qb;
#ifdef DEBUGSERIAL
        Serial.println("Begin: GRADIENT command parsing");
        Serial.println("Command: ");
        Serial.println(dataLine);
#endif

        //Get data
        ta = atof(strstr(dataLine, flagTA) + 2);    //get buff @ the index of the flag buff, then parse it and return a float.
        qa = atof(strstr(dataLine, flagQA) + 2);
        tb = atof(strstr(dataLine, flagTB) + 2);
        qb = atof(strstr(dataLine, flagQB) + 2);

#ifdef DEBUGSERIAL
        Serial.print("Command index: ");
        Serial.println(commandIndex);
        Serial.println(ta);
        Serial.println(qa);
        Serial.println(tb);
        Serial.println(qb);
#endif
        if (ta == 0 && commandIndex != 0) { //Cheap way of figuring out if String.toFloat failed
#ifdef DEBUGSERIAL
            Serial.println("This command has stopped the routine.");
#endif
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
                float newQ = (float) mapFloat((millis() - startTime), 0.0, (tb - ta) * 1000.0, qa, qb);
                setQ(newQ);
            }

            if ((millis() - startTime) > (tb - ta) * 1000) { //if the gradient is complete
                break;
            }
            doOpenLoopStuff();  //While we wait, do all the things.
        }
    } else {
#ifdef DEBUGSERIAL
        Serial.println("Begin: NORMAL command parsing");
        Serial.print("Command: ");
        Serial.println(dataLine);
#endif
        float t;
        float q;

        //Get data
#ifdef DEBUGSERIAL
        Serial.print("Data: ");
        Serial.println(strchr(dataLine, flagT) + 1);
#endif
        t = atof(strchr(dataLine, flagT) + 1);
        q = atof(strchr(dataLine, flagQ) + 1);
#ifdef DEBUGSERIAL
        Serial.print("Command index: ");
        Serial.println(commandIndex);
        Serial.println(t);
        Serial.println(q);
#endif
        if (t == 0 && commandIndex != 0) { //Cheap way of figuring out if String.toFloat failed
#ifdef DEBUGSERIAL
            Serial.println("This command has stopped the routine.");
#endif
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
