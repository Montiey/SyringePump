// Copyright 2019 Jason Harriot

#include "FlowStepper.h"

FlowStepper::FlowStepper(unsigned char s, unsigned char d){
	_stepPin = s;
	_dirPin = d;

	pinMode(_stepPin, OUTPUT);
	pinMode(_dirPin, OUTPUT);
	setDirection(1);
}

void FlowStepper::setDirection(int d){
	digitalWrite(_dirPin, d >= 0 ? 1 : 0);
}

void FlowStepper::setSpeed(float speed){
	_startTime = safeMicros();
	_startSpeed = speed;
	_startSpeedInterval = 1000000.0 / speed;
}

void FlowStepper::setAccel(float accel){
	_accelMicros = accel/1000000;
}

bool FlowStepper::runSpeed(){
	unsigned long long now = safeMicros();
	if(now - _lastStep >= _startSpeedInterval){
		_lastStep = now;
		digitalWrite(_stepPin, HIGH);
		delayMicroseconds(5);
		digitalWrite(_stepPin, LOW);

		return true;
	}

	return false;
}

bool FlowStepper::runAccel(){
	unsigned long long now = safeMicros();
	float speed = _startSpeed + ((now - _startTime) * (_accelMicros));
	unsigned long long intervalNow;

	if(speed < MINSPEED){
		intervalNow = ULLONG_MAX;
	} else{
		intervalNow = 1000000.0 / speed;
	}

	if(now - _lastStep >= intervalNow){
		_lastStep = now;
		digitalWrite(_stepPin, HIGH);
		delayMicroseconds(2);	//Minimum pulse duration
		digitalWrite(_stepPin, LOW);

		return true;
	}

	return false;
}
