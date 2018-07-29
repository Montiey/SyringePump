#define LEDG 2
#define LEDY 3
#define LEDR 4

void setup() {
    pinMode(LEDG, OUTPUT);
    pinMode(LEDY, OUTPUT);
    pinMode(LEDR, OUTPUT);
}

void loop() {
    digitalWrite(LEDG, HIGH);
    delay(250);
    digitalWrite(LEDY, HIGH);
    delay(250);
    digitalWrite(LEDR, HIGH);
    delay(250);
    digitalWrite(LEDG, LOW);
    delay(250);
    digitalWrite(LEDY, LOW);
    delay(250);
    digitalWrite(LEDR, LOW);
    delay(250);
}
