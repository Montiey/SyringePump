// Copyright 2019 Jason Harriot
#ifndef CONFIG_H
#define CONFIG_H

#include "MultiTimer.h"
#include "FlowStepper.h"

//Hardware Configuration
#define PITCH 0.8	//Pitch of the threaded rod
#define STEPS 400	//Steps / rev of the motor
#define USTEP_RATE 32	//The Microstepping rate to use while pumping (1, 2, 4, 8, 16, 32)
#define JOG_SPEED 950
#define JOG_USTEP 1
#define CMDFILE "commands.txt"
#define CONFIGFILE "config.txt"

//Misc Configuration
#define CONFIGSIZE 64	//Size of config to load
#define CMDLINESIZE 64
#define CMDLINES 20

#define DB_THRESH 35	//Minimum valid press duration
#define DB_REPEAT 125	//Minimum time between presses

#define DB_HOLD_THRESH 125	//Minimum valid hold time

//Hardware definitions
#define BUTTONSEL A3
#define BUTTONF A1
#define BUTTONR A2

#define STEP 5
#define DIR 9

#define SDCS 10

#define MODE0 8
#define MODE1 7
#define MODE2 6

#define LEDR 2
#define LEDG 4
#define LEDB 3

//Flags
#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7

#define TYPENOOP 0
#define TYPELINEAR 1
#define TYPEGRADIENT 2

#define PRINTDEC 4	//Decimals for debug prints

const char flagT = 't';
const char flagQ = 'q';
const char * const flagTA = "ta";
const char * const flagQA = "qa";
const char * const flagTB = "tb";
const char * const flagQB = "qb";
const char * const flagID = "id";
const char * const flagTUNE = "tune";

struct ConfigData{
	float diameter;
	float tune;
	float direction;
	float ULPerStep;
};

struct CommandFrame{
	unsigned char type;
	bool isLastOnDisk;
	float ta;
	float qa;
	float tb;
	float qb;

	CommandFrame(){
		type = TYPELINEAR;
		isLastOnDisk = false;
		ta = 0;
		qa = 0;
		tb = 0;
		qb = 0;
	}

	CommandFrame(unsigned char eType, bool eLast, float eTa, float eQa, float eTb, float eQb){
		type = eType;
		isLastOnDisk = eLast;
		ta = eTa;
		qa = eQa;
		tb = eTb;
		qb = eQa;
	}
};

extern FlowStepper stepper;
extern ConfigData config;

#endif
