// Copyright 2019 Jason Harriot
#include "MultiTimer.h"

unsigned long long lastSafeMicros;
unsigned long lastUnsafeMicros;

unsigned long long safeMicros(){	//Gracefully handles integer rollover
	unsigned long now = micros();

	lastSafeMicros += now - lastUnsafeMicros;	//safe comparison, delta always positive

	lastUnsafeMicros = now;

	return lastSafeMicros;
}

HandyTimer::HandyTimer(unsigned long a){
	interval = a;
}

bool HandyTimer::trigger(){
	checkTime = millis();
	if(checkTime - lastTime >= interval){
		lastTime = checkTime;
		return true;
	}
	return false;
}

bool HandyTimer::setInterval(unsigned long a){
	if(interval == a){
		return false;
	}
	interval = a;
	return true;
}
