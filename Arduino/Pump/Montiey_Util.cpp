#include <Arduino.h>
#include "Montiey_Util.h"

HandyTimer::HandyTimer(unsigned long a){
	interval = a;
}

bool HandyTimer::trigger(){
    if(disable) return false;
	checkTime = millis();
	if(checkTime - lastTime >= interval){
		lastTime = checkTime;
		return true;
	}
	return false;
}
bool HandyTimer::updateInterval(unsigned long a){
	if(interval == a){
		return false;
	}
	interval = a;
	return true;
}

MicroTimer::MicroTimer(unsigned long a){
    interval = a;
}

bool MicroTimer::trigger(){
    if(disable) return false;
    checkTime = micros();
    if(checkTime - lastTime >= interval){
        lastTime = checkTime;
        return true;
    }
    return false;
}
bool MicroTimer::updateInterval(unsigned long a){
    if(interval == a){
        return false;
    }
    interval = a;
    return true;
}
