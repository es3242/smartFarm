#include <config.h>  

//sda -> 21, sdl -> 22

const uint32_t FREQ = 25000;
const uint8_t  RES  = 10;

// pin info
const int DHTPIN = 5;       // DHT22 DATA → GPIO5
const int DHTTYPE = DHT22;

const int FAN_PIN = 26;  // ESP32 → MOSFET PWM+ 에 연결한 GPIO
const int FAN_ACTIVE_LOW = 25;

extern const int WATER_PUMP = 18;

const int SOIL_PIN = 34;

const int  servoPin = 13;

// 전역 (기본값은 현재 쓰는 값과 동일)
float OK_LOW  = 40.0f;
float WET_LOW = 70.0f;

const int NUM_SAMPLES = 8;

float g_tempC = NAN, g_humi = NAN;

//serco cali****
const int minUS = 600;   // 캘리브레이션으로 조정
const int maxUS = 2400;  // 캘리브레이션으로 조정
int servo_freq  = 50;

unsigned long g_lastDht = 0;
const unsigned long DHT_PERIOD = 2000; // 최소 2초 간격 권장

const int SCREEN_WIDTH =128;
const int SCREEN_HEIGHT =64;

const int pumpon_offset = 4; //time(sec) that pump takes to release water from the end;;