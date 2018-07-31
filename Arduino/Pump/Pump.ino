/// Copyright 2018 Jason Harriot
/// Units: Seconds, uL (mm3) / sec
/// General notes: Serial prints take alot of time, so plan on using the pump without them.

#include "AccelStepper.h"
#include "Montiey_Util.h"
#include <SPI.h>
#include <SD.h>

//////// vvv CONFIGURATION vvv ////////

/// Removes ALL serial commands (for use)
//#define NOSERIAL

#ifndef NOSERIAL

/// Enables spammy debugging prints (for testing) (if not already disabled above)
#define DEBUGSERIAL

#endif


//////////////////////


#define ID 18.5    //Inner diameter of the syringe
#define PITCH 0.8  //Pitch of the threaded rod
#define STEPS 400 //Steps per revolution of the motor
#define USTEP_RATE 32   //This sets digital pins to control microstepping (1, 2, 4, 8, 16, or 32)
#define TUNE 1  //Multiplier applied to step speed (2 = twice the fluid)
#define JOG_SPEED 3000   //Steps / sec for jog moves
#define MAX_LINE_BYTES 64   //max number of bytes to load for a command
#define BUTTONSEL A3    //select button
#define BUTTONF A2  //forward jog
#define BUTTONR A1  //reverse jog
#define LEDR 4
#define LEDY 3
#define LEDG 2
#define STEP 5  //STEP pin
#define DIR 9   //DIR pin
#define SDCS 10 //SPI chip select
#define MODE0 8 //µStep modes
#define MODE1 7
#define MODE2 6
#define CMDFILE "commands.txt"


////////////////
////////////////


File dataFile;
const float UL_PER_STEP = PITCH * (360.0 / (STEPS * USTEP_RATE) ) * (PI * (float)(pow(ID/2, 2)));   //mm3
const char flagT = 't'; //defined here because strstr and strchr don't like literals
const char flagQ = 'q';
const char * flagTA = "ta";
const char * flagQA = "qa";
const char * flagTB = "tb";
const char * flagQB = "qb";
char * dataLine = (char *) calloc(MAX_LINE_BYTES, 1);    //A single command line may contain MAX_LINE_BYTES characters. Increase if more memory is availible.
AccelStepper s(1, STEP, DIR); //1 = "driver mode" (operate with pulse and direction pins)
HandyTimer recalculationInterval(125); //How often to perform (lengthy) calculations and set the updated speed
int commandIndex = 0;   //Current line of text
unsigned long offsetTime = 0;   //Time at which the routine was started (with button)


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
	dataFile = SD.open(CMDFILE);
	if (!dataFile) {
		#ifndef NOSERIAL
		Serial.println("FILE not found");
		#endif
		endGame();
	}

	s.setMaxSpeed(1000000); //Something to do with acceleration, which we don't care about. Just use a very high number that we won't hit
	s.setSpeed(0);  //Don't move anything do start with

	while (digitalRead(BUTTONSEL)) {   //wait for the button to begin the routine
		doJog();
	}

	offsetTime = millis();
}

void loop() {
	everything();
}
