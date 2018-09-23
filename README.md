# SyringePump
Arduino-based liquid micromanipulation system

The source is built using Atom / PlatformIO. If compiled with the stock Arduino IDE, the libraries will need to be relocated (Recommended to use Atom).

If you want to tweak the firmware, download the source, and open Pump.ino in PlatformIO. Several #define statements control the pump's behavior.

SD Interface:
* commands.txt contains a line-by-line speed routine, where the pump changes to the speed specified at the time specified on that line.
* config.txt contains a parameter for the diameter of the syringe in use, as well as a multiplier. The Multiplier is used to precisely tune the ammount of liquid pumped.

Sample commands:
```
t0 q100	//First, set the flow rate to 100 uLiters/sec
t1 q200	//One second later, same thing...
t2 q300	//So on, but now we'll wait longer
t6 q0	//4 seconds after the last command, stop.
ta10 qa0 tb20 qb20	//Starting at 10 seconds and ending at 20 seconds, pump a gradient between 0 and 20 uL/sec
ta20 qa20 tb30 qb0	//Same thing, but in reverse.
t30 q20	//Set the speed as soon as the gradient ends
t35 q0	//Stop pumping (the program ends at the time specified by the last command, so logically it should always have a `q` of 0)
```

Sample config:
```
ID 4.5	//Inner diameter of the syringe
TN 1.0	//Flow multiplier (make negative to reverse the motor!)
```
