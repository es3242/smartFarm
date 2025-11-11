#include "fan.h"

static bool s_on=false;
static unsigned long s_changed=0;

void fan_init(){
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  s_on=false; 
  s_changed=millis();
}
void fan_on() {
  digitalWrite(FAN_PIN, HIGH);  // HIGH = ON
  s_on = true;
  s_changed = millis();
}

void fan_off() {
  digitalWrite(FAN_PIN, LOW);   // LOW = OFF
  s_on = false;
  s_changed = millis();
}
bool fan_is_on(){ return s_on; }

unsigned long fan_last_changed(){ return s_changed; }