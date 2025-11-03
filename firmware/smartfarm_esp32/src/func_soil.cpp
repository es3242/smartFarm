#include <func_soil.h>
#include <config.h>


int get_soilraw(){
      long acc = 0;
  for (int i = 0; i < NUM_SAMPLES; ++i) {
    acc += analogRead(SOIL_PIN);
    delay(5);
  }
  int raw = acc / NUM_SAMPLES;
  return raw;
}

int rawToPercent(int raw) {
  // 3550(건조)=0%, 1840(젖음)=100% → 선형보간
  const int DRY_RAW  = 3550;
  const int WET_RAW  = 1840;
  float pct = (float)(DRY_RAW - raw) * 100.0f / (DRY_RAW - WET_RAW);
  if (pct < 0)   pct = 0;
  if (pct > 100) pct = 100;
  return (int)(pct + 0.5f); // 반올림
}

const char* getSoilState(int percent){
  
  const char* state=
  (percent >= 70) ? "WET" :
  (percent >= 40) ? "OK " : "DRY";

  return state;
}