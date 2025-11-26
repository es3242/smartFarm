#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "oled_display.h"

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void oled_init() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 주소 0x3C가 대부분
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("SmartFarm booting"));
  display.display();
}

void oled_show_status(float soil_pct, bool pump_on, bool fan_on) {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.print(F("Soil: "));
  display.print(soil_pct, 0);
  display.println(F("%"));

  display.print(F("Pump: "));
  display.println(pump_on ? F("ON") : F("OFF"));

  display.print(F("Fan : "));
  display.println(fan_on ? F("ON") : F("OFF"));

  display.display();
}
