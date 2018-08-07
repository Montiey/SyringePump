# SyringePump
Arduino-based liquid micromanipulation interface

The source is built using Atom / PlatformIO. If compiled with the stock Arduino IDE, some libraries may have to be manually included (Recommended to use Atom).
Download the source, and open Pump.ino in PlatformIO. Several #define statements control the pump's behavior.

SD Interface:

-commands.txt contains a line-by-line speed routine, where the pump changes to the speed specified at the time specified on that line.

-config.txt contains a parameter for the diameter of the syringe in use, as well as a multiplier. The Multiplier is used to precisely tune the ammount of liquid pumped.

Sample commands:
```t0 q100
t1 q200
t2 q300
t3 q-300
t4 q-200
t5 q-100
t6 q0
```

Sample config:
```ID 4.5	//Inner diameter of the syringe
TN 1.0	//Tuning multiplier
```

[Model files](https://www.thingiverse.com/thing:3038896)
