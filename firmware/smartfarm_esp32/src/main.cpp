#include <Arduino.h>

#include "config.h"
#include "fan.h"
#include "water_pump.h"

#include "wifi_ota.h"
#include "micro_ros_node.h"
#include "log.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Booting...");

  // 하드웨어 초기화
  pinMode(FAN_PIN, OUTPUT);
  fan_off();
  pump_init();

  // 네트워크/OTA
  wifi_connect();           // WiFi + UDP 로그 시작
  ota_init(HOSTNAME);       // OTA 준비

  // micro-ROS 노드 초기화
  microRosInit();
}

void loop() {
  ota_handle();             // OTA 처리
  microRosSpinOnce();       // micro-ROS executor
  delay(10);
}
