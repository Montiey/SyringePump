#include <stdio.h>
#include "AccelStepper.h"
#include "Config.h"
#include <Arduino.h>

AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR);
ConfigData config;
