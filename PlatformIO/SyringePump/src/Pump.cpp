// Copyright 2019 Jason Harriot

#include "Config.h"
#include "HWControl.h"
#include "MotionControl.h"
#include <Arduino.h>

void setup() {
	setLED(WHITE);

	Serial.begin(250000);
	Serial.println("======== Montiey's Flow Pump V0.1 ========");

	pinMode(MODE0, OUTPUT);
	pinMode(MODE1, OUTPUT);
	pinMode(MODE2, OUTPUT);

	pinMode(SDCS, OUTPUT);	//SPI freaks out otherwise

	pinMode(LEDR, OUTPUT);
	pinMode(LEDG, OUTPUT);
	pinMode(LEDB, OUTPUT);

	pinMode(BUTTONSEL, INPUT);
	pinMode(BUTTONF, INPUT);
	pinMode(BUTTONR, INPUT);

	digitalWrite(BUTTONSEL, HIGH);	//Internal pullup (pressed = LOW)
	digitalWrite(BUTTONF, HIGH);
	digitalWrite(BUTTONR, HIGH);

	setLED(GREEN);
}

void loop() {
	enterJog();

	if(db(BUTTONSEL)){
		enterPump();
	}
}
