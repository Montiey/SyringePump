#define led LED_BUILTIN
#define button 12
const String sample = "t1.0 q400\nt2.0 qa200 e3.5 qb500";

int i = 0;
unsigned long offsetTime = 0;   //Time at which the routine was started

float flowRate = 0;

struct timer {
    unsigned long lastTime;
    unsigned long liveTime;
    unsigned long interval;
};

timer Pump;

String getLine(String s, int line) {
    if (line < 0) {
        return "";
    } else if (line == 0) {
        return s.substring(0, s.indexOf('\n'));
    } else {
        return getLine(s.substring(s.indexOf('\n') + 1), line - 1);
    }
}

void pump(int vel) {    //do stuff
    Pump.interval = 10000 / vel;
    digitalWrite(led, 1);
    delay(250);
    digitalWrite(led, 0);
    delay(10);
}

void doPump() {
    Pump.liveTime = millis();

    if (Pump.liveTime - Pump.lastTime > Pump.interval) {
        Pump.lastTime = Pump.liveTime;
        //        Serial.print(".");  //do stepper tick, etc.
    }
}

void endGame() {
    while (1) {
        Serial.println("END");
        delay(1000);
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
        unsigned long startTime = millis(); //start of the gradient
        while (1) {
            if (millis() - offsetTime >= t * 1000) { //if the queued event is up or has passed
                break;
            }
            doPump();   //Trigger steppers here, since it is 99.999% of runtime
        }
        while (1) {
            doPump();
            int newQ = map((millis() - startTime), 0.0, (e - t) * 1000.0, qa, qb);
            //do math here
            Pump.interval = newQ; //not real math, just for printing
            String temp = "Interval: ";
            Serial.println(temp + Pump.interval + (millis() - startTime) + (e-t) * 1000);

            if((millis() - startTime) >= (e-t) * 1000){ //if the gradient is complete
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
            doPump();   //Trigger steppers here, since it is 99.999% of runtime
        }
        pump(q);
    }

    i++;
}
