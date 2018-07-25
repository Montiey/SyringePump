#ifndef MONTIEY_UTIL_H
#define MONTIEY_UTIL_H

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

#endif
