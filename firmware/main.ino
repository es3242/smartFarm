#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>

#define DHTPIN   5        // DHT22 DATA → GPIO5

const int FAN_PIN = 26;  // ESP32 → MOSFET PWM+ 에 연결한 GPIO
const uint32_t FREQ = 25000;
const uint8_t  RES  = 10;

#define DHTTYPE  DHT22

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;  

float g_tempC = NAN, g_humi = NAN;
unsigned long g_lastDht = 0;
const unsigned long DHT_PERIOD = 2000; // 최소 2초 간격 권장
// --- DHT22 additions ---

// OLED 세팅 (네가 쓰는 값 유지)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 토양센서
const int SOIL_PIN = 34;
const int NUM_SAMPLES = 8;

// 네가 이미 가지고 있는 함수들 가정
int rawToPercent(int raw) {
  // 네 스케치에 있는 변환 로직 그대로 사용하면 됨
  // 예시:  (필요시 네가 쓰던 식으로 대체)
  // 4095(건조) -> 0%,  1500(젖음) -> 100% 등
  int p = map(raw, 3500, 1600, 0, 100); // 대략 예시값
  p = constrain(p, 0, 100);
  return p;
}
// 유틸
int textWidthPx(const String& s, uint8_t size=1) {
  return s.length() * 6 * size; // 기본폰트 6px 폭
}
void printRightAligned(Adafruit_SSD1306& d, int x_right, int y, const String& s, uint8_t size=1) {
  int w = textWidthPx(s, size);
  d.setTextSize(size);
  d.setCursor(x_right - w, y);
  d.print(s);
}

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  Serial.begin(115200);

  // ledcAttach(FAN_PIN, FREQ, RES);                 // 채널 지정 없이 핀에 직접 연결
  // ledcWrite(FAN_PIN, (1U << RES) - 1);            // 100% 듀티 (10bit면 1023)

  Wire.begin(21, 22);    // I2C 시작 (OLED와 동일 라인)
  Wire.setClock(100000); // I2C 속도 조정
  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed");
    for(;;);
  }
  display.clearDisplay();
  display.display();

  dht.begin();
  lightMeter.begin();    // [추가] BH1750 초기화

  analogReadResolution(12);  // ESP32 기본 12bit
}

void drawBar(int percent) {
  // 네가 이미 쓰는 바 그리기 함수 그대로 사용
  // (없다면 간단 예시)
  int w = map(percent, 0, 100, 0, 120);
  display.drawRect(4, 54, 120, 8, WHITE);
  display.fillRect(4, 54, w, 8, WHITE);
}

/*
void drawBar(int percent) {
  int w = map(percent, 0, 100, 0, 120);
  display.drawRect(4, 60, 120, 4, WHITE);   // 막대 위치/높이(겹침 방지)
  display.fillRect(4, 60, w, 4, WHITE);
}
*/

void loop() {
  digitalWrite(FAN_PIN, HIGH);   // 팬 켜기
  Serial.println("Fan ON");
  delay(3000);                   // 3초 대기

  digitalWrite(FAN_PIN, LOW);    // 팬 끄기
  Serial.println("Fan OFF");
  delay(3000);                   // 3초 대기
  
  // ---- 토양 평균샘플 ----
  long acc = 0;
  for (int i = 0; i < NUM_SAMPLES; ++i) {
    acc += analogRead(SOIL_PIN);
    delay(5);
  }
  int raw = acc / NUM_SAMPLES;
  int percent = rawToPercent(raw);

  // 상태 문구 (네 스케치 로직 유지)
  const char* state =
      (percent >= 70) ? "WET" :
      (percent >= 40) ? "OK " : "DRY";

  // --- DHT22 additions --- (비차단 주기 측정)
  unsigned long now = millis();
  if (now - g_lastDht >= DHT_PERIOD) {
    float h = dht.readHumidity();
    float t = dht.readTemperature(); // 섭씨
    if (!isnan(h)) g_humi = h;
    if (!isnan(t)) g_tempC = t;
    g_lastDht = now;
  }
  // --- DHT22 additions ---

    // ---- BH1750 조도 ----
  float lux = lightMeter.readLightLevel();          // lx
  if (isnan(lux)) lux = 0;                          // 방어
  if (lux > 99999) lux = 99999;                     // 표기 범위 제한


  // ---- 시리얼 출력  ----
  Serial.print("RAW:"); Serial.print(raw);
  Serial.print("  Moisture:"); Serial.print(percent); Serial.print("%  State:"); Serial.print(state);
  Serial.print("  Temp:"); Serial.print(isnan(g_tempC)? -999 : g_tempC);
  Serial.print("C  Humi:"); Serial.print(isnan(g_humi)? -999 : g_humi); Serial.print("%");
  Serial.print("  Lux:"); Serial.println(lux, 1);

  // --- DHT22 additions ---

  // ---- OLED ----
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(WHITE, BLACK);

  // ---- 제목 (크기2, 10자 이내) ----
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Soil/DHT");  // 10자 이하 제목

  // ---- 2열 레이아웃 (크기1) ----
  // 왼쪽 라벨 고정, 오른쪽 값은 우측 정렬
  const int Lx = 0;
  const int Rx = 126;
  const int Y1 = 16, Y2 = 28, Y3 = 40, Y4 = 48;   // 4번째 줄을 48로
  const int BAR_Y = 60;                            // 상태 바 위치
  const int BAR_H = 4;                            // 막대 높이(60~63)

  // 1행 (y=16)
  display.setTextSize(1);
  display.setCursor(Lx, 16);
  display.print("RAW");
  printRightAligned(display, Rx, 16, String(raw), 1);

  // 2행 (y=28)
  display.setCursor(Lx, 28);
  display.print("Moist");
  printRightAligned(display, Rx, 28, String(percent) + "%", 1);

  // 3행 (y=40)
  display.setCursor(Lx, 40);
  display.print("State");
  {
    String st = String(state);
    if (st.length() > 10) st = st.substring(0, 10); // 너무 길면 컷
    printRightAligned(display, Rx, 40, st, 1);
  }

  // 4행(아래줄) ― y=48로 올림
  display.setCursor(Lx, Y4);
  display.print("T/H");
  {
    String th;
    if (isnan(g_tempC) || isnan(g_humi)) th = "--/--";
    else th = String(g_tempC, 1) + "C " + String((int)g_humi) + "%";
    // 너무 길면 우측이 잘리니 필요 시 더 줄이기
    printRightAligned(display, Rx, Y4, th, 1);
  }

  // ---- 하단 바(56~63px 영역) ----
  int barW = map(percent, 0, 100, 0, 120);
  display.drawRect(4, BAR_Y, 120, BAR_H, WHITE);
  display.fillRect(4, BAR_Y, barW, BAR_H, WHITE);

  display.display();

  delay(500); // DHT는 millis로 2초 주기라 이 딜레이 유지해도 OK
}
