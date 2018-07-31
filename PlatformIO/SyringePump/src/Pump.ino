/// Copyright 2018 Jason Harriot
/// Units: Seconds, uL (mm3) / sec

#include "AccelStepper.h"
#include "MultiTimer.h"
#include <SD.h>

//////// vvv CONFIGURATION vvv ////////

/// Removes ALL serial commands (for use)
#define NOSERIAL

#ifndef NOSERIAL

/// Enables spammy debugging prints (for testing) (if not already disabled above)
#define DEBUGSERIAL

#endif


//// Tuning / customization:

#define ID 18.5    //Inner diameter of the syringe
#define PITCH 0.8  //Pitch of the threaded rod
#define STEPS 400 //Steps per revolution of the motor
#define USTEP_RATE 32   //This sets digital pins to control microstepping (1, 2, 4, 8, 16, or 32)
#define TUNE 1  //Multiplier applied to step speed (2 = twice the fluid)
#define JOG_SPEED 1000   //Steps / sec for jog moves
#define JOG_USTEP 2	//microstepping for jog moves
#define MAX_LINE_BYTES 64   //max number of bytes to load for a command (increase if neccesary to exceed)
#define CMDFILE "commands.txt"


//// Hardware definitions:

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

//// Miscelaneous:

unsigned long lastSel = 0;
#define DB_THRESH 25	//debouncing time (less than the fastest possible valid button push)
#define PUMPMODE 0
#define JOGMODE 1
#define ERRORMODE 2
File dataFile;
const float UL_PER_STEP = PITCH * (360.0 / (STEPS * USTEP_RATE) ) * (PI * (float)(pow(ID/2, 2)));   //mm3
const char flagT = 't'; //defined here because strstr and strchr don't like literals
const char flagQ = 'q';
const char * flagTA = "ta";
const char * flagQA = "qa";
const char * flagTB = "tb";
const char * flagQB = "qb";
char * dataLine = (char *) calloc(MAX_LINE_BYTES, 1);    //A single command line may contain MAX_LINE_BYTES characters.
AccelStepper s(1, STEP, DIR); //1 = "driver mode" (operate with pulse and direction pins)
HandyTimer recalculationInterval(125); //How often to perform (lengthy) calculations and set the updated speed
int commandIndex = 0;   //Current line of text
unsigned long offsetTime = 0;   //Time at which the routine was started (with button)
byte runMode = 0;	//variable to hold the current mode at any given time


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
	pinMode(SDCS, OUTPUT);	//set CS pin as output, SPI no likey otherwise
	pinMode(LEDR, OUTPUT);
	pinMode(LEDG, OUTPUT);
	pinMode(LEDY, OUTPUT);
	pinMode(BUTTONSEL, INPUT);
	pinMode(BUTTONF, INPUT);
	pinMode(BUTTONR, INPUT);
	digitalWrite(BUTTONSEL, HIGH); //internal pullup (pressed = LOW)
	digitalWrite(BUTTONF, HIGH);
	digitalWrite(BUTTONR, HIGH);

	initSD();

	s.setMaxSpeed(1000000); //Acceleration ceiling, which we don't care about. Just use a very high number that we won't hit
	s.setSpeed(0);  //Don't move anything do start with
	runMode = JOGMODE;
}

void loop() {
	if(runMode == PUMPMODE){
		setLED(1, 0, 0);
		setStepRate(USTEP_RATE);
		if(pump() == -1){	//If the pumping routine has finished
			runMode = JOGMODE;

			#ifndef NOSERIAL
			Serial.println("Pumping routine finished");
			#endif
		}
	} else if(runMode == JOGMODE){
		setLED(0, 1, 0);
		setStepRate(JOG_USTEP);
		if(jogWithButtons() == -1){	//if we should start the pumping routine
			#ifndef NOSERIAL
			Serial.println("Jog routine finished");
			#endif
			runMode = PUMPMODE;
			commandIndex = 0;
			offsetTime = millis();	//pump() runs "immediately" after this, so we can set it here without loss of timing.
			initSD();
		}
	}
}
