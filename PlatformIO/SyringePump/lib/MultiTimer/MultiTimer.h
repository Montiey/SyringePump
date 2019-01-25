#ifndef MULTITIMER_H
#define MULTITIMER_H

class HandyTimer {
public:
	HandyTimer(unsigned long);
	bool trigger();
	bool updateInterval(unsigned long);
	unsigned long interval;
	bool disable;
private:
	unsigned long checkTime;
	unsigned long lastTime;
};

class MicroTimer {
public:
	MicroTimer(unsigned long);
	bool trigger();
	bool updateInterval(unsigned long);
	unsigned long interval;
	bool disable;
private:
	unsigned long checkTime;
	unsigned long lastTime;
};

#endif
