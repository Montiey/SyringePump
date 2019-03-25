// Copyright 2019 Jason Harriot
#ifndef MULTITIMER_H
#define MULTITIMER_H

#include <Arduino.h>

unsigned long long safeMicros();

class HandyTimer {
public:
	HandyTimer(unsigned long);
	bool trigger();
	bool setInterval(unsigned long);
	unsigned long interval;
private:
	unsigned long checkTime;
	unsigned long lastTime;
};

#endif
