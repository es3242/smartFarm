#pragma once
#include <Arduino.h>

void oled_init();
void oled_show_status(float soil_pct, bool pump_on, bool fan_on);
