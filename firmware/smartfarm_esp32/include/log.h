#pragma once
#include <Arduino.h>

void log_init();                    // WiFi 붙은 뒤에 한 번 호출
void logLine(const String& msg);    // 어디서나 사용 가능
