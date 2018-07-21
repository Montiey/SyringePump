/// Copyright 2018 Jason Harriot
/// Standard units: Seconds, uL / sec


#define led A4
#define button 2
#define UL_PER_STEP 0.01973325157   // Calculated value
const String sample = "t1.0 q400\nt2.0 qa0 e12.0 qb100000";

int i = 0;
unsigned long offsetTime = 0;   //Time at which the routine was started (with button)

float flowRate = 0;

struct timer {
    unsigned long lastTime;
    unsigned long liveTime;
    unsigned long interval;
};

timer Pump;

void(* reset)(void) = 0;

String getLine(String s, int line) {
    if (s.lastIndexOf('\n') > 0) {  //make sure that the string ends with a newline, otherwise everything loops
        s = s + "\n";
    }
    if (line < 0) {
        return "";
    } else if (line == 0) {
        return s.substring(0, s.indexOf('\n'));
    } else {
        return getLine(s.substring(s.indexOf('\n') + 1), line - 1);
    }
}

float getStepInterval(float q) {    // UL/sec -> 1/(steps/sec) = secs/step
    return 1.0 / map(q, 0, 1.0, 0, 1.0 / UL_PER_STEP);
}

void doPump() {
    Pump.liveTime = millis();

    if (Pump.liveTime - Pump.lastTime > Pump.interval) {
        Pump.lastTime = Pump.liveTime;
        //        Serial.print(".");  //do stepper tick, etc.

        //
        digitalWrite(led, 1);
        delay(50);                  // <<< represents blocking required to make a stepper pulse
        digitalWrite(led, 0);
        //
    }
}

void endGame() {
    Serial.println("END");
    while (1) {
        if (!digitalRead(button)) {
            reset();
        }
    }
}

void setup() {
    Serial.begin(250000);
    pinMode(led, OUTPUT);
    pinMode(button, INPUT);
    digitalWrite(button, HIGH);

    while (digitalRead(button)) {
        offsetTime = millis();
    }
}

void loop() {
    String cmd = getLine(sample, i);

    float t;
    float q;
    float qa;
    float e;
    float qb;

    String a = "Next event: ";
    if (cmd.indexOf("qa") > -1) {   //if command is for gradient
        t = cmd.substring(cmd.indexOf('t') + 1).toFloat();
        qa = cmd.substring(cmd.indexOf("qa") + 1).toFloat();
        e = cmd.substring(cmd.indexOf('e') + 1).toFloat();
        qb = cmd.substring(cmd.indexOf("qb") + 1).toFloat();
        Serial.println(a + i); Serial.println(t); Serial.println(q); Serial.println(qa); Serial.println(e); Serial.println(qb);

        if (t == 0) endGame();

        while (1) {
            if (millis() - offsetTime >= t * 1000) { //if the queued event is up or has passed
                break;
            }
            //This is still the last setPump, just waiting for the right time to trigger the gradient.
            doPump();   //Trigger steppers here, since it is 99.999% of runtime
        }

        unsigned long startTime = offsetTime + (t * 1000); //start of the gradient where it SHOULD be, not where control is
        while (1) {
            doPump();
            float newQ = map((millis() - startTime), 0.0, (e - t) * 1000.0, qa, qb);  // q at any point across the gradient
            //do math here
            Pump.interval = getStepInterval(newQ); //not real math, just for printing
            String temp = "Interval: ";
            String temp2 = "Live: ";
            String temp3 = "Target: ";
            Serial.println(temp + Pump.interval);
            Serial.println(temp2 + (millis() - startTime));
            Serial.println(temp3 + (e - t) * 1000);
            if ((millis() - startTime) >= (e - t) * 1000) { //if the gradient is complete
                break;
            }
        }
    } else {
        t = cmd.substring(cmd.indexOf('t') + 1).toFloat();
        q = cmd.substring(cmd.indexOf('q') + 1).toFloat();
        Serial.println(a + i); Serial.println(t); Serial.println(q);

        if (t == 0) endGame();

        while (1) {
            if (millis() - offsetTime >= t * 1000) { //if the queued event is up or has passed
                break;
            }
            //Still the last pump, just waiting.
            doPump();   //Trigger steppers here, since it is 99.999% of runtime
        }
        Pump.interval = getStepInterval(q);
    }

    i++;
}
