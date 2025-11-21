#include "fan.h"

bool           fan_on_flag       = false;
unsigned long  fan_s_changed     = 0;

// 1시간에 20분 ON
// 덴버 시간 기준 팬 스케줄
static const int FAN_START_HOUR = 8;                 // 08:00부터
static const int FAN_END_HOUR   = 20;                // 20:00까지 (20시는 제외)
static const int FAN_ON_SECONDS_IN_HOUR = 10 * 60;   // 매시 첫 10분(=600초)

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

void fan_auto_update() {
  if (fan_mode != FAN_MODE_AUTO) return;

  time_t now;
  time(&now);

  struct tm t;
  if (localtime_r(&now, &t) == nullptr) {
    // 아직 시간 동기화가 안 되었거나 에러일 때 → 안전하게 OFF
    if (fan_hw_state) {
      fan_hw_state = false;
      fan_on_flag  = false;
      fan_off();
      fan_s_changed = millis();
    }
    return;
  }

  int hour = t.tm_hour;  // 0~23 (덴버 기준, wifi_ota에서 TZ 세팅함)
  int min  = t.tm_min;   // 0~59
  int sec  = t.tm_sec;   // 0~59

  // 08:00 ~ 20:00 사이인지
  bool in_active_hours = (hour >= FAN_START_HOUR && hour < FAN_END_HOUR);

  // 매시 첫 10분(0~599초)인지
  int sec_in_hour = min * 60 + sec;
  bool in_first_10min = (sec_in_hour < FAN_ON_SECONDS_IN_HOUR);

  bool should_on = in_active_hours && in_first_10min;

  if (should_on != fan_hw_state) {
    fan_hw_state = should_on;
    fan_on_flag  = should_on;
    fan_s_changed = millis();   // 상태 변경 시각 기록

    if (should_on) {
      fan_on();
      logLine("FAN AUTO ON");
    } else {
      fan_off();
      logLine("FAN AUTO OFF");
    }
  }
}
