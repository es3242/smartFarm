// fan.h
#pragma once
void fan_init();           // pinMode 등 1회

void fan_on();

void fan_off();
bool fan_is_on();

unsigned long fan_last_changed();