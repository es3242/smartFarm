#include<web.h>

String contentTypeFor(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".htm")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".jpg")) return "image/jpeg";
    else if (filename.endsWith(".jpeg")) return "image/jpeg";
    else if (filename.endsWith(".gif")) return "image/gif";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".xml")) return "text/xml";
    else if (filename.endsWith(".pdf")) return "application/pdf";
    else if (filename.endsWith(".zip")) return "application/zip";
    else if (filename.endsWith(".gz")) return "application/x-gzip";
    else if (filename.endsWith(".json")) return "application/json";
    return "text/plain";
}

void setupRoutes() {
  server.on("/hello", [](){
    server.send(200, "text/plain", "Hello World");
  });
}

void handleFileRead(const String& pathReq){
  String path = pathReq;
  if(path.endsWith("/")) path += "index.html";
  if(LittleFS.exists(path)){
    File f = LittleFS.open(path, "r");
    server.streamFile(f, contentTypeFor(path));
    f.close();
  }else{
    server.send(404, "text/plain", "Not found");
  }
}

void handleStatus(){
  // 1) ADC 읽기
  int raw = analogRead(SOIL_PIN);

  // 2) 퍼센트 계산 (네 캘리브레이션 값으로 수정 가능)
  const int DRY = 3500, WET = 1600;        // 예시
  int clamped = constrain(raw, WET, DRY);
  float soil_pct = 100.0f * float(DRY - clamped) / float(DRY - WET);

  // 3) 상태 라벨
  const char* soil_state =
      (soil_pct >= WET_LOW) ? "WET" :
      (soil_pct >= OK_LOW)  ? "OK"  : "DRY";


  // 4) JSON 응답 (모두 소수 포함)
  char buf[256];
  snprintf(buf, sizeof(buf),
    "{\"temp\":%.2f,\"hum\":%.2f,"
    "\"soil_pct\":%.1f,\"soil_raw\":%d,\"soil_state\":\"%s\"}",
    isnan(g_tempC)?-999:g_tempC,
    isnan(g_humi)?-999:g_humi,
    soil_pct, raw, soil_state
  );
  server.send(200, "application/json; charset=utf-8", buf);
}

void handleAct(){
  if(server.hasArg("fan")){
    int v = server.arg("fan").toInt();
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(FAN_PIN, v?HIGH:LOW);
  }
  server.send(204);
}
// 노트 읽기/쓰기
String readNote(){
  if(!LittleFS.exists("/note.txt")) return "";
  File f = LittleFS.open("/note.txt","r");
  String s = f.readString(); f.close(); return s;
}
void writeNote(const String& s){
  File f = LittleFS.open("/note.txt","w");
  f.print(s); f.close();
}

// /cfg GET: 현재 경계값 + 노트 제공
void handleCfgGet(){
  String note = readNote();
  char buf[512];
  snprintf(buf, sizeof(buf),
    "{\"ok_low\":%.1f,\"wet_low\":%.1f,\"note\":%s}",
    OK_LOW, WET_LOW,
    String("\""+note+"\"").c_str() // 간단히 처리(따옴표 포함 주의 필요시 json 이스케이프 보강)
  );
  server.send(200, "application/json; charset=utf-8", buf);
}

// /cfg POST: ok_low, wet_low, note 저장
void handleCfgPost(){
  if(server.hasArg("ok_low"))  OK_LOW  = server.arg("ok_low").toFloat();
  if(server.hasArg("wet_low")) WET_LOW = server.arg("wet_low").toFloat();
  if(server.hasArg("note"))    writeNote(server.arg("note"));
  server.send(204);
}

