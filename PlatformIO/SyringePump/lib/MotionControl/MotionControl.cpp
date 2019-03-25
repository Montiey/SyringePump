// Copyright 2019 Jason Harriot
#include "Config.h"
#include "HWControl.h"
#include "SDControl.h"
#include <Arduino.h>

unsigned long doMotor(CommandFrame current, CommandFrame next){
	unsigned long duration;
	float speed1;
	float speed2;

	switch(current.type){
		case TYPELINEAR:
			duration = (next.ta - current.ta) * 1000;

			speed1 = current.qa / config.ULPerStep;
			stepper.setSpeed(speed1);

			Serial.print("Running: ");
			Serial.print(speed1);
			Serial.print(" For: ");
			Serial.println(duration / 1000.0, PRINTDEC);
		break;
		case TYPEGRADIENT:
			stepper.setSpeed(current.qa / config.ULPerStep);

			duration = (current.tb - current.ta) * 1000;

			speed1 = current.qa / config.ULPerStep;
			speed2 = current.qb / config.ULPerStep;

			float a = (speed2 - speed1) / (duration / 1000.0);
			stepper.setAccel(a);

			Serial.print("Running: ");
			Serial.print(speed1, PRINTDEC);
			Serial.print(" > ");
			Serial.print(speed2, PRINTDEC);
			Serial.print(" For: ");
			Serial.println(duration / 1000.0, PRINTDEC);
		break;
	}

	return duration;
}

void enterPump(){
	HandyTimer LEDBlink(500);
	HandyTimer stat(500);

	pullSDConfig();
	dumpCommands();

	CommandFrame currentCommand;
	CommandFrame nextCommand;
	pullCMDFrame(0, currentCommand);
	pullCMDFrame(1, nextCommand);

	unsigned int currentIndex = 1;

	setStepping(USTEP_RATE);
	stepper.setDirection(1);

	unsigned long setDuration = doMotor(currentCommand, nextCommand);

	unsigned long start = millis();

	while(1){
		if(LEDBlink.trigger()){
			setLED(getColor() == BLACK ? BLUE : BLACK);
		}
		if(db(BUTTONSEL)){
			while(button(BUTTONSEL));
			break;
		}

		switch(currentCommand.type){
			case TYPELINEAR:
			stepper.runSpeed();
			break;
			case TYPEGRADIENT:
			stepper.runAccel();
			break;
		}

		if(millis() - start >= setDuration){
			currentCommand = nextCommand;

			if(currentCommand.isLastOnDisk){	//Handle the last command
				if(currentCommand.type == TYPEGRADIENT){	//Don't stop immediately if it's a gradient
					nextCommand = CommandFrame(TYPELINEAR, true, currentCommand.tb, 0, 0, 0);
				} else{	//Stop if it's a linear command, however.
					break;
				}
			} else{	//Just get the next command
				currentIndex++;
				pullCMDFrame(currentIndex, nextCommand);
			}

			setDuration = doMotor(currentCommand, nextCommand);

			start = millis();
		}
	}

	Serial.println("Pump stopped.");
}

bool jogFHeld = false;
bool jogRHeld = false;

void enterJog(){
	if(button(BUTTONR)) {
		if(!jogRHeld && !jogFHeld && dbHold(BUTTONR)){
			jogRHeld = true;	//Skip hold check for next presses
			stepper.setDirection(-1);
			stepper.setSpeed(JOG_SPEED);
			setStepping(JOG_USTEP);
			setLED(WHITE);
		}
	} else{
		jogRHeld = false;
	}

	if(button(BUTTONF)) {
		if(!jogFHeld && !jogRHeld && dbHold(BUTTONF)){
			jogFHeld = true;
			stepper.setDirection(1);
			stepper.setSpeed(JOG_SPEED);
			setStepping(JOG_USTEP);
			setLED(WHITE);
		}
	} else{
		jogFHeld = false;
	}

	if(jogFHeld || jogRHeld) stepper.runSpeed();
	else setLED(GREEN);
}
