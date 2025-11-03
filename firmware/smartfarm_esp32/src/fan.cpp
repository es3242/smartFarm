// fan.cpp
#include <Arduino.h>
#include "config.h"
#include "fan.h"

static bool s_on=false;
static unsigned long s_changed=0;

static inline int LV(bool on){
  return FAN_ACTIVE_LOW ? (on?LOW:HIGH) : (on?HIGH:LOW);
}
void fan_init(){
  digitalWrite(FAN_PIN, LV(false));
  pinMode(FAN_PIN, OUTPUT);
  s_on=false; s_changed=millis();
}
void fan_write(bool on){
  s_on=on; s_changed=millis();
  digitalWrite(FAN_PIN, LV(on));
}
bool fan_is_on(){ return s_on; }
unsigned long fan_last_changed(){ return s_changed; }