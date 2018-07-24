#include "AccelStepper.h"
#include "Montiey_Util.h"

#define STEP 13
#define DIR 2
#define STEPS_PER_REV 400

HandyTimer t(10);

AccelStepper s(1, STEP, DIR);

int x = 0;
bool asc = true;
#define a 20

void setup() {
    s.setMaxSpeed(10000);
    s.setSpeed(x);
}

void loop() {
    if(t.trigger()){
        if(asc){
            x += a;
            if(x > 10000){
                asc = false;
            }
        } else{
            x -= a;
            if(x < 0){
                asc = true;
            }
        }
        s.setSpeed(x);
    }

    
    s.runSpeed();
    
}

