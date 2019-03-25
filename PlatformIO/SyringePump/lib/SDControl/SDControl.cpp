// Copyright 2019 Jason Harriot
#include <Arduino.h>
#include <unistd.h>
#include <SD.h>
#include "SDControl.h"
#include "Math.h"
#include "Config.h"

void pullSDConfig(){	//Pull the entire config into the data struct
	SD.begin(SDCS);

	if(!SD.exists(CONFIGFILE)){
		Serial.println("Error reading config file");
	}

	File f = SD.open(CONFIGFILE);

	char * buf = (char *) calloc(CONFIGSIZE, sizeof(char));

	for(unsigned int i = 0; i < CONFIGSIZE; i++){
		if(!f.available()) break;
		buf[i] = f.read();
	}

	config.diameter = atof(strstr(buf, flagID)+strlen(flagID));
	config.tune = atof(strstr(buf, flagTUNE)+strlen(flagTUNE));
	config.direction = config.tune >= 0 ? 1 : -1;

	Serial.print("Pitch (mm / rev): ");
	Serial.println(PITCH);
	Serial.print("Microstepping rate: ");
	Serial.println(USTEP_RATE);
	Serial.print("Motor resolution (full-steps / rev): ");
	Serial.println(STEPS);

	config.ULPerStep = config.tune * M_PI * pow(config.diameter/2, 2) * (PITCH / (USTEP_RATE * STEPS));
	// config.ULPerStep = (PITCH / (USTEP_RATE * STEPS));

	Serial.print("Config file: Diameter=");
	Serial.print(config.diameter);
	Serial.print(" Tune=");
	Serial.print(config.tune);
	Serial.print(" (Direction=");
	Serial.print(config.direction == 1 ? "FWD" : "REV");
	Serial.print(" ULPerStep=");
	Serial.print(config.ULPerStep, PRINTDEC);
	Serial.println(")");

	free(buf);
	f.close();
}

float _getFlagValue(const char * f, char * s){	//Get the value of a (string) flag in a string
	char * index = strstr(s, f)+strlen(f);
	return atof(index == NULL ? (char *) "0" : index);
}

float _getFlagValueChar(const char f, char * s){	//Get the value of a (character) flag in a string
	char * index = strchr(s, f)+1;
	return atof(index == NULL ? (char *) "0" : index);
}

bool pullCMDLine(char * buf, unsigned int line){	//Pull a line from the SD card
	// Serial.print("Pulling from line: ");
	// Serial.println(line);

	bool succ = true;
	SD.begin(SDCS);
	File f = SD.open(CMDFILE);

	for(unsigned int i = 0; i < line; i++){	//Find a newline i times
		if(!f.available()){
			succ = false;
			break;
		}

		char c = f.read();

		while(c != '\n'){
			if(!f.available()){
				succ = false;
				break;
			}
			c = f.read();	//Ignore up to and including the next newline, or EOF
		}
	}

	if(succ) for(unsigned int i = 0; i < CMDLINESIZE; i++){	//After the newline, put the next line into the buffer
		if(!f.available()){
			succ = false;
			break;
		}

		buf[i] = f.read();

		if(buf[i] == '\n') break;
	}

	f.close();

	// Serial.print("Read buffer: ");
	// Serial.println(buf);

	return succ;
}

void pullCMDFrame(unsigned int line, CommandFrame & ret){	//Get a CommandFrame from a specific line on disk

	char * buf = (char *) calloc(CMDLINESIZE, sizeof(char));

	for(int j = 0; j < CMDLINESIZE; j++) buf[j] = 0;	//Clear the buffer of the last read.

	if(pullCMDLine(buf, line)){
		// Serial.println("Dump succeeded");
	} else{
		ret.isLastOnDisk = true;
	}

	if(strstr(buf, flagTA) != NULL && strstr(buf, flagQA) != NULL && strstr(buf, flagTB) != NULL && strstr(buf, flagQB) != NULL){
		// Serial.println("Gradient command");
		ret.type = TYPEGRADIENT;
		ret.ta = _getFlagValue(flagTA, buf);
		ret.qa = _getFlagValue(flagQA, buf);
		ret.tb = _getFlagValue(flagTB, buf);
		ret.qb = _getFlagValue(flagQB, buf);
	} else if(strchr(buf, flagT) != NULL && strchr(buf, flagQ) != NULL){
		// Serial.println("Linear command");
		ret.type = TYPELINEAR;
		ret.ta = _getFlagValueChar(flagT, buf);
		ret.qa = _getFlagValueChar(flagQ, buf);
	} else{
		ret.type = TYPENOOP;
		Serial.print("Syntax error on line: ");
		Serial.print(line + 1);
	}

	free(buf);
}

void dumpCMDFrame(CommandFrame f, unsigned int index){	//Print a command
	Serial.print(index);
	Serial.print(": ");

	const char * t;
	switch(f.type){
		case TYPELINEAR: t = "L"; break;
		case TYPEGRADIENT: t = "G"; break;
		default: t = "X"; break;
	}
	Serial.print(t);

	Serial.print(" |\t");
	Serial.print(f.ta);
	Serial.print("\t|\t");
	Serial.print(f.qa);
	Serial.print("\t|\t");
	Serial.print(f.tb);
	Serial.print("\t|\t");
	Serial.print(f.qb);
	Serial.print("\t|\t");
	Serial.println(f.isLastOnDisk ? "LAST" : "----");
}

void dumpCommands(){
	for(unsigned int i = 0; i >= 0; i++){
		CommandFrame f;
		pullCMDFrame(i, f);
		dumpCMDFrame(f, i);
		if(f.isLastOnDisk){
			break;
		}
	}
}
