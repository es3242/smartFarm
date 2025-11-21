#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "keys.h"      // wifi_ssid, wifi_pwd
#include "log.h"
#include "config.h"

void wifi_connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pwd);

  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // UDP 로그 시작
  log_init();
}

void ota_init(const char* hostname) {
  ArduinoOTA.setHostname(hostname);

  ArduinoOTA.onStart([]() { logLine("Start OTA"); });
  ArduinoOTA.onEnd([]() { logLine("End OTA"); });
  ArduinoOTA.onError([](ota_error_t error) {
    logLine("OTA Error[" + String((uint32_t)error) + "]");
  });

  ArduinoOTA.begin();
  logLine("OTA ready");
}

void ota_handle() {
  ArduinoOTA.handle();
}
