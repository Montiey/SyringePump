// Copyright 2019 Jason Harriot

#ifndef SDCONTROL_H
#define SDCONTROL_H

#include "Config.h"

void pullSDConfig();
void pullCMDFrame(unsigned int, CommandFrame &);
bool pullCMDLine(char *, unsigned int);

void dumpCommands();
void dumpCMDFrame(CommandFrame f, unsigned int);
#endif
