#include <Arduino.h>
#include "Montiey_Util.h"

bool clampBounds(int &val, int low, int high){
	if(val >= high){
		val = high;
		return true;
	}
	if(val <= low){
		val = low;
		return true;
	}
	return false;
}
bool clampBounds(unsigned char &val, unsigned char low, unsigned char high){
	if(val >= high){
		val = high;
		return true;
	}
	if(val <= low){
		val = low;
		return true;
	}
	return false;
}
bool clampBounds(char &val, char low, char high){
	if(val >= high){
		val = high;
		return true;
	}
	if(val <= low){
		val = low;
		return true;
	}
	return false;
}

void null(){}

void zero(double &val, double vel){
	if(val > 0) val -= vel;
	else val += vel;
	if(abs(val) - abs(vel) <= 0) val = 0;
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
bool HandyTimer::updateInterval(unsigned long a){
	if(interval == a){
		return false;
	}
	interval = a;
	return true;
}

LaggyTimer::LaggyTimer(unsigned long a){
	interval = a;
}
unsigned long LaggyTimer::trigger(void(*updateFunction)()){
	checkTime = millis();
	if(checkTime - lastTime >= interval){
		unsigned long num = (checkTime - lastTime) / interval;

		for(unsigned long i = 0; i < num; i++){
			// Serial.print(i);
			// Serial.print(" / ");
			// Serial.println(num+1);
			updateFunction();
		}
		lastTime = checkTime;
		return num;
	}
	return 0;
}

unsigned long LaggyTimer::trigger(){
	checkTime = millis();
	if(checkTime - lastTime >= interval){
		unsigned long num = (checkTime - lastTime) / interval;
		lastTime = checkTime;
		return num;
	}
	return 0;
}
