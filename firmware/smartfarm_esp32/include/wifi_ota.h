#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <time.h>   

#include "keys.h"      // wifi_ssid, wifi_pwd
#include "log.h"
#include "config.h"


void wifi_connect();          // WiFi 붙을 때까지 blocking
void ota_init(const char* hostname);
void ota_handle();            // loop()에서 계속 호출
