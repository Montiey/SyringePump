// Copyright 2019 Jason Harriot

#include <Arduino.h>
#include <Arduino.h>
#include "MultiTimer.h"

#ifndef FLOWSTEPPER_H
#define FLOWSTEPPER_H

#define ULLONG_MAX 18446744073709551615U
#define MINSPEED (1000000.0 / ULLONG_MAX )

class FlowStepper{
public:
	FlowStepper(unsigned char, unsigned char);
	void setDirection(int);
	void setSpeed(float);	//Steps / sec
	void setAccel(float);	//Steps / sec^2
	bool runSpeed();
	bool runAccel();
private:
	unsigned char _stepPin;
	unsigned char _dirPin;

	float _startSpeed;
	unsigned long long _startTime;
	unsigned long long _startSpeedInterval;
	unsigned long long _lastStep;
	float _accelMicros;	//Steps/uSec^2
};


#endif
