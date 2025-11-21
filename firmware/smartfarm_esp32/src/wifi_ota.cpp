#include "wifi_ota.h"

void wifi_connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pwd);

  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP: ");
  Serial.println(WiFi.localIP());

  // UDP 로그 시작 (먼저)
  log_init();
  logLine("WiFi connected, IP: " + WiFi.localIP().toString());

  // ===== NTP / 로컬 시간 동기화 (America/Denver 기준) =====
  configTzTime("MST7MDT,M3.2.0/2,M11.1.0/2",
               "pool.ntp.org", "time.google.com");

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    logLine("Failed to obtain time from NTP");
  } else {
    char buf[64];
    // timeinfo를 문자열로 포맷팅
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);

    logLine(String("Time sync OK: ") + buf);
    // 시리얼로도 보고 싶으면:
    // Serial.println(buf);
  }
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
