#include <water_pump.h>
#define SEC(x) ((x) * 1000)

static bool p_on=false;
static unsigned long p_changed=0;

void pump_init(){
  pinMode(WATER_PUMP, OUTPUT);
  digitalWrite(WATER_PUMP, LOW); // 시작은 꺼진 상태
  p_on=false; 
  p_changed=millis();
}
void pump_on() {
  digitalWrite(WATER_PUMP, HIGH);  // HIGH = ON
  p_on = true;
  p_changed = millis();
}

void pump_off() {
  digitalWrite(WATER_PUMP, LOW);   // LOW = OFF
  p_on = false;
  p_changed = millis();
}

void pump_test() {
    pump_on();
    Serial.print("pump_on");
    delay(pumpon_offset + SEC(5));
    pump_off();
    Serial.print("pump_off");
    delay(50000);
}
bool pump_ip_on(){ return p_on; }

unsigned long pump_last_changed(){ return p_changed; }