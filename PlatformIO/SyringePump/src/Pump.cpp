/// Copyright 2018 Jason Harriot
/// Units: Seconds, uL (mm3) / sec

#include "AccelStepper.h"
#include "MultiTimer.h"
#include "SDControl.h"
#include "Config.h"
#include "HWControl.h"
#include "MotionControl.h"
#include <Arduino.h>

void setup() {
	setLED(WHITE);

	delay(2000);
	Serial.begin(115200);
	Serial.println("======== Montiey's Flow Pump V0.1 ========");

	pinMode(MODE0, OUTPUT);
	pinMode(MODE1, OUTPUT);
	pinMode(MODE2, OUTPUT);
	pinMode(SDCS, OUTPUT);	//Set CS pin as output, SPI no likey otherwise
	pinMode(LEDR, OUTPUT);
	pinMode(LEDG, OUTPUT);
	pinMode(LEDB, OUTPUT);
	pinMode(BUTTONSEL, INPUT);
	pinMode(BUTTONF, INPUT);
	pinMode(BUTTONR, INPUT);
	digitalWrite(BUTTONSEL, HIGH);	//Internal pullup (pressed = LOW)
	digitalWrite(BUTTONF, HIGH);
	digitalWrite(BUTTONR, HIGH);

	pullSDConfig();

	// Serial.print("Config: ");
	// Serial.print(config.diameter);
	// Serial.print(" ");
	// Serial.print(config.tune);
	// Serial.print(" ");
	// Serial.print(config.direction);
	// Serial.print(" ");
	// Serial.println(config.ULPerStep);

	// for(unsigned int i = 0; i >= 0; i++){
	// 	CommandFrame f;
	// 	pullLineFrame(i, f);
	// 	dumpCommandFrame(f, i);
	// 	if(f.isLastOnDisk){
	// 		Serial.println("That was the last one");
	// 		break;
	// 	}
	// }

	stepper.setMaxSpeed(1000);
	setLED(GREEN);
}

void loop() {
	enterJog();

	if(db(BUTTONSEL)){
		enterPump();
	}
}
