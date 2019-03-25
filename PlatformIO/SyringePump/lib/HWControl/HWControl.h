// Copyright 2019 Jason Harriot
#ifndef HWCONTROL_H
#define HWCONTROL_H

void setStepping(unsigned char);
void setLED(unsigned char);

unsigned char getColor();

bool db(unsigned char);
bool dbHold(unsigned char);

bool button(unsigned char);

#endif
