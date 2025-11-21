#pragma once

void wifi_connect();          // WiFi 붙을 때까지 blocking
void ota_init(const char* hostname);
void ota_handle();            // loop()에서 계속 호출
