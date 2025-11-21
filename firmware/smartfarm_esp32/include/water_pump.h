#pragma once
#include <Arduino.h>
#include "config.h"


void pump_init();
void pump_on();

void pump_off();
bool pump_ip_on();
void pump_test();

unsigned long pump_last_changed();