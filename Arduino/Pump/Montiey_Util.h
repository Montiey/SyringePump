#ifndef MONTIEY_UTIL_H
#define MONTIEY_UTIL_H

bool clampBounds(int&, int, int);
bool clampBounds(unsigned char&, unsigned char, unsigned char);
bool clampBounds(char&, char, char);

void null();

void zero(double&, double);

class HandyTimer {
	public:
		HandyTimer(unsigned long);
		bool trigger();
		bool updateInterval(unsigned long);
		unsigned long interval;
	private:
		unsigned long checkTime;
		unsigned long lastTime;
};

class LaggyTimer {
	public:
		LaggyTimer(unsigned long);
		unsigned long trigger(void(*)());
		unsigned long trigger();
		unsigned long interval;
	private:
		unsigned long checkTime;
		unsigned long lastTime;
};

#endif
