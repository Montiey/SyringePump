# SyringePump
Arduino-based liquid micromanipulation interface


Objective:

Control liquid flow with a simple interface, with the ability to specify precise routines and timing circumstances.

Interface:

Plaintext is written to a file and loaded via SD card by the microcontroller. 3 buttons are used to manually control the pump and / or execute the commands from disk.

Setup:

The project is built using Atom / PlatformIO. If compiled with the stock Arduino IDE, some libraries may have to be manually included.
Download the source, and open Pump.ino in PlatformIO. Several #define statements control the pump's behavior.

ID should be set to the inner diameter of the syringe (measure with calipers).

PITCH should be set to the pitch of the threaded rod (0.8 for most M5).

STEPS steps per rotation (usually 200 or 400).

USTEP_RATE sets the driver pins to control microstepping. Using values other than 1, 2, 4, 8, 16 or 32 will result in full-stepping.

TUNE is a multiplier calculated internally on top of stepping speed. Use for precisely tuning the flow rate. Defaults to 1.

JOG_SPEED sets steps / sec for jog moves.

JOG_USTEP sets the driver pins for microstepping only while in jog mode.

MAX_LINE_BYTES is a limit on how many bytes will be buffered from the SD card per line. Default of 64 is unlikely to be exceeded.

CMDFILE is the name of the file to read from. This file must be at the root of the card. Case and file extension sensitive.
