#include <stdio.h>
#include "Config.h"
#include "HWControl.h"
#include "AccelStepper.h"
#include "MultiTimer.h"
#include "SDControl.h"
#include "Util.h"
#include <Arduino.h>

bool jogHeld = false;

void setStepper(float time, float speed){	//Internal function; Given a time interval in seconds and a speed in uL/sec, set a TARGET POSITION in steps.
	Serial.print(speed, PRINTDEC);
	Serial.print(" for ");
	Serial.print(time, PRINTDEC);

	float stepSpeed = speed / config.ULPerStep;

	stepper.move(time * stepSpeed);
	stepper.setSpeed(stepSpeed);
	if(stepper.speed() < stepSpeed);

	Serial.print(" Target: ");
	Serial.print(stepper.targetPosition());

	Serial.print(" Speed: ");
	Serial.println(stepper.speed(), PRINTDEC);
}

unsigned long start = 0;

void enterPump(){
	HandyTimer t(100);
	HandyTimer stat(1000);

	unsigned int currentIndex = 0;

	CommandFrame nextCommand;
	pullLineFrame(1, nextCommand);
	CommandFrame currentCommand;
	pullLineFrame(0, currentCommand);

	stepper.setCurrentPosition(0);
	setStepping(USTEP_RATE);

	setStepper(nextCommand.ta - currentCommand.ta, currentCommand.qa);

	start = millis();

	while(1){
		if(t.trigger()){
			setLED(getColor() == RED ? BLUE : RED);
		}
		// if(stat.trigger()){
		// 	Serial.print("To go: ");
		// 	Serial.print(stepper.distanceToGo());
		// 	Serial.println();
		// }
		if(db(BUTTONSEL)){
			while(button(BUTTONSEL));
			return;
		}

		///

		stepper.runSpeedToPosition();

		if(stepper.distanceToGo() == 0){
			unsigned long stop = millis();
			Serial.print("End of command. Elapsed time: ");
			Serial.println((stop - start) / 1000.0, PRINTDEC);

			if(currentCommand.isLastOnDisk){
				printf("End of command cycle!");
				break;
			}
			currentIndex++;
			currentCommand = nextCommand;
			pullLineFrame(currentIndex, nextCommand);
			switch(currentCommand.type){
				case TYPENOOP:

				break;
				case TYPELINEAR:
					setStepper(nextCommand.ta - currentCommand.ta, currentCommand.qa);
				break;
				case TYPEGRADIENT:

				break;
				default:
				Serial.println("Invalid command type");
				break;
			}
			start = millis();
		}
	}
}

void enterJog(){
	if(button(BUTTONR)) {
		if(!jogHeld){
			if(dbHold(BUTTONR)) jogHeld = true;	//Skip hold check for next presses
			setStepping(JOG_USTEP);
		} else{
			stepper.setSpeed(-JOG_SPEED * config.direction);
			setLED(YELLOW);
		}
	} else if(button(BUTTONF)) {
		if(!jogHeld){
			if(dbHold(BUTTONF)) jogHeld = true;	//Skip hold check for next presses
			setStepping(JOG_USTEP);
		} else{
			stepper.setSpeed(JOG_SPEED * config.direction);
			setLED(YELLOW);
		}
	} else{	//If no buttons
		stepper.setSpeed(0);
		setLED(GREEN);
		jogHeld = false;	//Recheck the buttons for holding next time
	}

	if(jogHeld) stepper.runSpeed();
}
