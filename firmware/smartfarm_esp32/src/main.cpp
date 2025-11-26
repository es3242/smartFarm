#include "main.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Booting...");

  // 하드웨어 초기화
  fan_init();
  pump_init();

  // 네트워크/OTA
  wifi_connect();           // WiFi + UDP 로그 시작
  ota_init(HOSTNAME);       // OTA 준비

  oled_init(); 
  // micro-ROS 노드 초기화
  microRosInit();
}

void loop() {
  ota_handle();             // OTA 처리
  microRosSpinOnce();       // micro-ROS executor
  delay(10);
}
