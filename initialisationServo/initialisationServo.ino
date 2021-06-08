#include <Servo.h>

void setup() {
        Serial.begin(9600);
        Serial.println();

        Servo monservo;
        monservo.attach(0);
        Serial.println("Servo en position initiale :");
        Serial.println(monservo.read());
        Serial.println();
        delay(2000);
        monservo.write(0);
        Serial.println("Servo en position 0 :");
        Serial.println(monservo.read());
        Serial.println("Terminé");
}

void loop() {}
