#pragma once
#include <DHT.h>  // DHT22 매크로 정의 포함
#include <Arduino.h> 

extern const uint32_t FREQ;
extern const uint8_t  RES;

// pin info
extern const int DHTPIN;       // DHT22 DATA → GPIO5
extern const int DHTTYPE;

extern const int FAN_PIN;  // ESP32 → MOSFET PWM+ 에 연결한 GPIO
extern const int FAN_ACTIVE_LOW;

extern const int SOIL_PIN;

extern const int  servoPin;

// 전역 (기본값은 현재 쓰는 값과 동일)
extern float OK_LOW;
extern float WET_LOW;

extern const int NUM_SAMPLES;

extern float g_tempC;
extern float g_humi;

//serco cali****
extern const int minUS ;   // 캘리브레이션으로 조정
extern const int maxUS ;  // 캘리브레이션으로 조정
extern int servo_freq  ;

extern unsigned long g_lastDht ;
extern const unsigned long DHT_PERIOD ; // 최소 2초 간격 권장
// --- DHT22 additions ---

// OLED 세팅 
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;