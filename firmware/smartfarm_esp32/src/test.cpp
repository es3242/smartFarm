#include <ESP32Servo.h>
#include <Arduino.h>
#include <test.h>
#include <config.h>


void servoTest(Servo& servo, int angle) {
   servo.write(0);
  delay(1000);
  servo.write(angle);
  delay(1000);
  servo.write(angle);
  delay(1000);
}

void servoTest(Servo& servo) {
   servo.write(0);
  delay(800);
  // servo.write(90);
  // delay(10);
  servo.write(180);
  delay(800);
}

void fanTest() {
  digitalWrite(FAN_PIN, HIGH);   // 팬 켜기
  Serial.println("Fan ON");
  delay(3000);                   // 3초 대기

  digitalWrite(FAN_PIN, LOW);    // 팬 끄기
  Serial.println("Fan OFF");
  delay(3000);                   // 3초 대기
}