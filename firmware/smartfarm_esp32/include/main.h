#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <web.h>
#include <WiFi.h>
#include <keys.h>
#include <fan.h>
#include <test.h>
#include <config.h>
#include <func_soil.h>

// #include <Servo.h>


int rawToPercent(int raw);
void setup();
void drawBar(int percent);
void servoTest(int angle);
