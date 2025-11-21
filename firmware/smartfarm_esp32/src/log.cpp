#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "keys.h"    // LOG_SERVER_IP, LOG_SERVER_PORT 들어있다고 가정
#include "log.h"

static WiFiUDP logUdp;
static bool loggingReady = false;

void logLine(const String& msg) {
  Serial.println(msg);

  if (!loggingReady || WiFi.status() != WL_CONNECTED) return;

  logUdp.beginPacket(LOG_SERVER_IP, LOG_SERVER_PORT);
  const char* cmsg = msg.c_str();
  logUdp.write((const uint8_t*)cmsg, msg.length());
  logUdp.write('\n');
  logUdp.endPacket();
}

void log_init() {
  // WiFi 연결된 뒤에 호출해야 함
  logUdp.begin(0);
  loggingReady = true;

  logLine("WiFi connected, IP: " + WiFi.localIP().toString());
}
