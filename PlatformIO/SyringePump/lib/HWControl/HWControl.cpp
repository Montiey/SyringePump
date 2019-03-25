// Copyright 2019 Jason Harriot
#include <Arduino.h>
#include "Config.h"

unsigned long lastSel = 0;

unsigned char currentColor = BLACK;

void setLED(unsigned char color){
	currentColor = color;
	bool r = 0;
	bool g = 0;
	bool b = 0;
	switch(color){
		case RED:
		r = 1;
		break;
		case GREEN:
		g = 1;
		break;
		case YELLOW:
		r = 1; g = 1;
		break;
		case BLUE:
		b = 1;
		break;
		case MAGENTA:
		r = 1; b = 1;
		break;
		case CYAN:
		g = 1; b = 1;
		break;
		case WHITE:
		r = 1; g = 1; b = 1;
		break;
	}

	digitalWrite(LEDR, !r);
	digitalWrite(LEDG, !g);
	digitalWrite(LEDB, !b);
}

unsigned char getColor(){
	return currentColor;
}

void _setMode(bool p0, bool p1, bool p2) {
	digitalWrite(MODE0, p0);
	digitalWrite(MODE1, p1);
	digitalWrite(MODE2, p2);
}

void setStepping(unsigned char rate){
	// Serial.print("Set uStepping: ");
	// Serial.println(rate);
	switch(rate) {
		case 1:
		_setMode(0, 0, 0);
		break;
		case 2:
		_setMode(1, 0, 0);
		break;
		case 4:
		_setMode(0, 1, 0);
		break;
		case 8:
		_setMode(1, 1, 0);
		break;
		case 16:
		_setMode(0, 0, 1);
		break;
		case 32:
		_setMode(1, 1, 1);
		break;
		default:
		_setMode(0, 0, 0);
		break;
	}
}

bool button(unsigned char pin){
	return !digitalRead(pin);
}

bool db(byte pin){	//Button debounce (WAITS FOR RELEASE)
	if(millis() - lastSel < DB_REPEAT){
		return false;	//Don't accept if pushed too fast since last
	}
	if(button(pin)){
		lastSel = millis();
		setLED(WHITE);
		while(button(pin)){

		}
		setLED(BLACK);
		if(millis() - lastSel < DB_THRESH){	//If not enough time has passed
			return false;
		} else{
			lastSel = millis();	//Successful button press
			return true;
		}
	}
	return false;
}

bool dbHold(byte pin){	//Button hold debounce (DOESN'T WAIT FOR RELEASE)
	unsigned long start = millis();
	while(button(pin)){
		if(millis() - start >= DB_HOLD_THRESH){
			return true;
		}
	}
	return false;
}
