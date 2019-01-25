#ifndef SDCONTROL_H
#define SDCONTROL_H

#include "Config.h"

void pullSDConfig();
void pullLineFrame(unsigned int, CommandFrame &);
bool getCMDLine(char *, unsigned int);

void dumpCommands(CommandFrame[]);
void dumpCommandFrame(CommandFrame f, unsigned int);
#endif
