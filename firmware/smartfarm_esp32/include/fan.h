#include <Arduino.h>
#include "config.h"

#pragma once
void fan_init();           // pinMode 등 1회

void fan_on();

void fan_off();
bool fan_is_on();

unsigned long fan_last_changed();

extern bool           fan_on_flag;         // 현재 논리 상태 (ON/OFF)
extern unsigned long  fan_s_changed;       // 마지막 상태 변경 시각(ms)

// 주기 설정 (1시간에 20분 ON)
extern const uint32_t FAN_CYCLE_PERIOD_MS ;  // 1시간
extern const uint32_t FAN_ON_DURATION_MS ;  // 20분

extern uint32_t       fan_cycle_start_ms;    // 기준 시각
extern bool           fan_hw_state;          // 실제 출력 상태(핀 상태)


// === 팬 자동 운전 설정 ===
enum FanMode {
  FAN_MODE_MANUAL,   // ROS에서 온 값 그대로 따라감
  FAN_MODE_AUTO    // 1시간 중 20분 자동으로 켰다 끔
};

extern FanMode fan_mode;

unsigned long fan_last_changed();

// millis() 기반 자동 팬 제어
void fan_auto_update();
