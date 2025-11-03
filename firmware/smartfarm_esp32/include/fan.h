// fan.h
#pragma once
void fan_init();           // pinMode 등 1회
void fan_write(bool on);   // 즉시 ON/OFF
bool fan_is_on();
