#include "fan.h"

bool           fan_on_flag       = false;
unsigned long  fan_s_changed     = 0;

// 1시간에 20분 ON
// 필요하면 config.h 쪽으로 옮겨도 됨
const uint32_t FAN_CYCLE_PERIOD_MS = 60UL * 60UL * 1000UL;   // 1시간
const uint32_t FAN_ON_DURATION_MS  = 20UL * 60UL * 1000UL;   // 20분

uint32_t       fan_cycle_start_ms = 0;       // 기준 시각
bool           fan_hw_state       = false;   // 실제 출력 상태

FanMode        fan_mode           = FAN_MODE_AUTO; // 기본은 AUTO 모드

void fan_init(){
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  fan_on_flag=false; 
  fan_s_changed=millis();
}
void fan_on() {
  digitalWrite(FAN_PIN, HIGH);  // HIGH = ON
  fan_on_flag = true;
  fan_s_changed = millis();
}

void fan_off() {
  digitalWrite(FAN_PIN, LOW);   // LOW = OFF
  fan_on_flag = false;
  fan_s_changed = millis();
}
bool fan_is_fan_on_flag(){ return fan_on_flag; }

unsigned long fan_last_changed(){ return fan_s_changed; }

// millis() 기반 자동 팬 제어
void fan_auto_update() {
  if (fan_mode != FAN_MODE_AUTO) return;

  uint32_t now = millis();
  // 부팅 이후 경과 시간 기준으로 주기 계산
  uint32_t elapsed_in_cycle = (now - fan_cycle_start_ms) % FAN_CYCLE_PERIOD_MS;

  bool should_on = (elapsed_in_cycle < FAN_ON_DURATION_MS);  // 앞 20분 ON, 나머지 40분 OFF

  if (should_on != fan_hw_state) {
    fan_hw_state = should_on;
    fan_on_flag  = should_on;

    if (should_on) fan_on();
    else           fan_off();
  }
}
