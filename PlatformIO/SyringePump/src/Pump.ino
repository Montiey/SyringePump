/// Copyright 2018 Jason Harriot
/// Units: Seconds, uL (mm3) / sec

#include "AccelStepper.h"
#include "MultiTimer.h"
#include <SD.h>

//////// vvv CONFIGURATION vvv ////////

/// Removes ALL serial commands (for regular use)
//#define NOSERIAL

#ifndef NOSERIAL
#define NODEBUG	/// Removes spammy prints only
#endif

////	Tuning / setup / customization:
#define PITCH 0.8	//Pitch of the threaded rod
#define STEPS 400	//Steps per revolution of the motor
#define USTEP_RATE 32	//This sets digital pins to control microstepping (1, 2, 4, 8, 16, or 32)
#define JOG_SPEED 1700	//Steps / sec for jog moves
#define JOG_USTEP 2	//Microstepping for jog moves
#define CMDFILE "commands.txt"
#define CONFIGFILE "config.txt"

//// Hardware definitions:

#define BUTTONSEL A3	//Select button
#define BUTTONF A1	//forward jog
#define BUTTONR A2	//Reverse jog
#define LEDR 2
#define LEDG 4
#define LEDB 3
#define STEP 5	//STEP pin
#define DIR 9	//DIR pin
#define SDCS 10	//SPI chip select
#define MODE0 8	//µStep modes
#define MODE1 7
#define MODE2 6

#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7
byte currentColor = 0;	//Value carryover for nested states. Always up to date (if using setLED())

//// Miscelaneous:

unsigned long lastSel = 0;	//Time of the last button press for debouncing
#define MAX_LINE_BYTES 64	//Max number of bytes to load for a single command
#define MAX_CONFIG_BYTES 32	//Maximum size (total) of the config file.
#define DB_THRESH 40	//Debouncing time (less than the fastest possible valid button push)
#define PUMPMODE 0
#define JOGMODE 1
#define ERRORMODE 2
File dataFile;
float ULPerStep;
const char flagT = 't';	//Defined here because strstr and strchr don't like literals
const char flagQ = 'q';
const char * flagTA = "ta";
const char * flagQA = "qa";
const char * flagTB = "tb";
const char * flagQB = "qb";
const char * flagID = "ID";
const char * flagTN = "TN";
char * dataLine = (char *) calloc(MAX_LINE_BYTES, 1);	//A single command line may contain MAX_LINE_BYTES characters or less.
AccelStepper s(1, STEP, DIR);	//1 = "driver mode" (operate with STEP and DIR pins)
HandyTimer recalculationInterval(250);	//How often to perform (lengthy) calculations and set the updated speed for gradients
HandyTimer LEDTimer(125);
int commandIndex = 0;	//Current line of text
unsigned long offsetTime = 0;	//Time at which the routine was started (with button)
byte runMode = 0;	//Variable to hold the current mode at any given time
float configID;	//Global value for the ID parameter (once loaded from SD)
float configTN;	//Global value for the TUNE parameter (once loaded from SD)
float configTNR;	//-1 or 1. Set in initSD(). For convenience.

////////////////
////////////////

void setup() {
	#ifndef NOSERIAL
	Serial.begin(115200);
	while(!Serial);
	#endif

	#ifndef NODEBUG
	Serial.println("======== Begin ========");
	#endif

	pinMode(MODE0, OUTPUT);
	pinMode(MODE1, OUTPUT);
	pinMode(MODE2, OUTPUT);
	pinMode(SDCS, OUTPUT);	//Set CS pin as output, SPI no likey otherwise
	pinMode(LEDR, OUTPUT);
	pinMode(LEDG, OUTPUT);
	pinMode(LEDB, OUTPUT);
	pinMode(BUTTONSEL, INPUT);
	pinMode(BUTTONF, INPUT);
	pinMode(BUTTONR, INPUT);
	digitalWrite(BUTTONSEL, HIGH);	//Internal pullup (pressed = LOW)
	digitalWrite(BUTTONF, HIGH);
	digitalWrite(BUTTONR, HIGH);

	initSD();

	s.setMaxSpeed(1000000);	//Acceleration ceiling, which we don't care about. Just use a very high number that we won't hit
	s.setSpeed(0);	//Don't move anything do start with

	runMode = JOGMODE;
	setStepRate(JOG_USTEP);
}

void loop() {
	if(runMode == PUMPMODE){
		setLED(GREEN);
		if(pump() == -1){	//If the pumping routine has finished
			runMode = JOGMODE;
			setStepRate(JOG_USTEP);
			#ifndef NOSERIAL
			Serial.println("Pumping routine finished");
			#endif
		}
	} else if(runMode == JOGMODE){
		setLED(BLUE);
		s.runSpeed();
		if(jogWithButtons() == -1){	//If we should start the pumping routine
			runMode = PUMPMODE;
			setStepRate(USTEP_RATE);
			#ifndef NOSERIAL
			Serial.println("Jog routine finished");
			#endif

			commandIndex = 0;
			initSD();
			offsetTime = millis();	//Set the time marker for the beginning of the routine
		}
	}
}
