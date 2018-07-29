/// Copyright 2018 Jason Harriot
/// Units: Seconds, uL (mm3) / sec
/// General notes: Serial prints take alot of time, so plan on using the pump without them.

//////// vvv USER CONFIGURATION vvv ////////

////////////////////////
/// Uncomment #define NOSERIALto remove ALL serial commands.
//#define NOSERIAL//////
////////////////////////

#ifndef NOSERIAL
//////////////////////
/// Comment out #define DEBUGSERIAL to remove the majority of spam serial prints
#define DEBUGSERIAL//////
//////////////////////
#endif


//////////////////////
#define ID 18.5    //Inner diameter of the syringe
#define PITCH 0.8  //Pitch of the threaded rod
#define STEPS 400 //Steps per revolution of the motor
#define USTEP_RATE 32   //Set this to the mirostepping setting of the driver


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
#define LED LED_BUILTIN
#define BUTTON 5
#define STEP 4
#define DIR 2

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
    while (!digitalRead(BUTTON));   //do a little debouncing
    delay(100);
    while (digitalRead(BUTTON));
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
    s.setSpeed(stepSpeed);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
#ifndef NOSERIAL
    Serial.begin(115200);
    Serial.print("µL per step: ");
    Serial.println(UL_PER_STEP);
#endif
#ifdef DEBUGSERIAL
    Serial.println("======== Begin ========");
#endif
    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    digitalWrite(BUTTON, HIGH); //internal pullup

    if (!SD.begin()) {
#ifndef NOSERIAL
        Serial.println("SD card not present");
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

    s.setMaxSpeed(1000000); //Something to do with acceleration, which we don't care about. Just use a very high number.
    s.setSpeed(0);  //Don't move anything do start with

    while (digitalRead(BUTTON)) {   //wait for the button to begin the routine
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
