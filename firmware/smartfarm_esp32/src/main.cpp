// #include <WiFi.h>
// #include <ArduinoOTA.h>
// #include <micro_ros_arduino.h>
// #include <rcl/rcl.h>
// #include <rclc/rclc.h>
// #include <rclc/executor.h>

// #include <std_msgs/msg/float32.h>
// #include <std_msgs/msg/bool.h>
// #include <std_msgs/msg/int32.h>

// #include "keys.h"        // wifi_ssid, wifi_pwd
// #include "config.h"      // 핀/임계값들(팬/펌프/토양센서 등)
// #include "func_soil.h"   // get_soilraw(), rawToPercent() 등
// #include "fan.h"         // fan_init(), fan_on/off() 등
// #include "water_pump.h"  // pump_init(), pump_on/off() 등

// // --- 네트워크/Agent ---
// #define AGENT_IP   "192.168.1.46"     // 데스크톱(Agent) IP
// #define AGENT_PORT 8888
// #define HOSTNAME   "shelf-mid"
// #define NS         "smartfarm/mid"

// // --- ROS 객체 ---
// rclc_support_t support;
// rcl_allocator_t allocator;
// rcl_node_t node;
// rclc_executor_t exec;

// rcl_publisher_t pub_soil;
// rcl_publisher_t pub_pump_state;
// rcl_publisher_t pub_fan_state;
// rcl_publisher_t pub_hb;
// rcl_subscription_t sub_pump_cmd;
// rcl_subscription_t sub_fan_cmd;

// rcl_timer_t t_soil, t_hb;

// std_msgs__msg__Float32 msg_soil;
// std_msgs__msg__Bool    msg_pump, msg_fan;
// std_msgs__msg__Int32  msg_hb;

// uint32_t hb_seq = 0;
// bool pump_on_flag = false;
// bool fan_on_flag  = false;

// // --- Wi-Fi/OTA ---
// static void wifi_connect() {
//   WiFi.mode(WIFI_STA);
//   WiFi.setHostname(HOSTNAME);
//   WiFi.setSleep(false);
//   WiFi.begin(wifi_ssid, wifi_pwd);
//   while (WiFi.status() != WL_CONNECTED) delay(120);
// }
// static void ota_begin() {
//   ArduinoOTA.setHostname(HOSTNAME);
//   ArduinoOTA.setPort(3232);
//   ArduinoOTA.begin();
// }

// // --- 센서 읽기 ---
// static float read_soil_pct() {
//   int raw = get_soilraw();
//   int pct = rawToPercent(raw);
//   return (float)pct;
// }

// // --- 타이머 콜백 ---
// static void cb_soil(rcl_timer_t*, int64_t) {
//   msg_soil.data = read_soil_pct();
//   rcl_publish(&pub_soil, &msg_soil, NULL);

//   // 상태 동기화(선택)
//   msg_pump.data = pump_on_flag;
//   rcl_publish(&pub_pump_state, &msg_pump, NULL);
//   msg_fan.data = fan_on_flag;
//   rcl_publish(&pub_fan_state, &msg_fan, NULL);
// }
// static void cb_hb(rcl_timer_t*, int64_t) {
//   msg_hb.data = hb_seq++;
//   rcl_publish(&pub_hb, &msg_hb, NULL);
// }

// // --- 구독 콜백 ---
// static void cb_pump(const void* msgin) {
//   auto* m = (const std_msgs__msg__Bool*)msgin;
//   pump_on_flag = m->data;
//   if (pump_on_flag) pump_on(); else pump_off();
//   msg_pump.data = pump_on_flag;
//   rcl_publish(&pub_pump_state, &msg_pump, NULL);
// }
// static void cb_fan(const void* msgin) {
//   auto* m = (const std_msgs__msg__Bool*)msgin;
//   fan_on_flag = m->data;
//   if (fan_on_flag) fan_on(); else fan_off();
//   msg_fan.data = fan_on_flag;
//   rcl_publish(&pub_fan_state, &msg_fan, NULL);
// }

// // --- 재연결 보조 ---
// static bool agent_alive() {
//   return rmw_uros_ping_agent(100, 1) == RMW_RET_OK;
// }
// static void ros_teardown() {
//   rclc_executor_fini(&exec);
//   rcl_publisher_fini(&pub_soil, &node);
//   rcl_publisher_fini(&pub_pump_state, &node);
//   rcl_publisher_fini(&pub_fan_state, &node);
//   rcl_publisher_fini(&pub_hb, &node);
//   rcl_subscription_fini(&sub_pump_cmd, &node);
//   rcl_subscription_fini(&sub_fan_cmd, &node);
//   rcl_timer_fini(&t_soil);
//   rcl_timer_fini(&t_hb);
//   rcl_node_fini(&node);
//   rclc_support_fini(&support);
// }
// static void ros_begin() {
//   set_microros_wifi_transports(wifi_ssid, wifi_pwd, AGENT_IP, AGENT_PORT);

//   allocator = rcl_get_default_allocator();
//   rclc_support_init(&support, 0, NULL, &allocator);

//   rcl_node_options_t nopt = rcl_node_get_default_options();
//   rclc_node_init_default(&node, HOSTNAME, NS, &support);

//   // QoS
//   rcl_publisher_options_t rel = rcl_publisher_get_default_options();
//   rel.qos.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
//   rcl_publisher_options_t be  = rcl_publisher_get_default_options();
//   be.qos.reliability  = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;

// // --- QoS 설정용 변수들 삭제 ---
// // rcl_publisher_options_t rel = rcl_publisher_get_default_options();
// // rcl_publisher_options_t be  = rcl_publisher_get_default_options();
// // rel.qos.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
// // be.qos.reliability  = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;

// // --- publishers ---
// rclc_publisher_init_best_effort(
//   &pub_soil, &node,
//   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
//   "soil_moisture");

// rclc_publisher_init_default(
//   &pub_pump_state, &node,
//   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
//   "pump_state");

// rclc_publisher_init_default(
//   &pub_fan_state, &node,
//   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
//   "fan_state");

// rclc_publisher_init_best_effort(
//   &pub_hb, &node,
//   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
//   "heartbeat");

// // --- subscriptions ---
// rclc_subscription_init_default(
//   &sub_pump_cmd, &node,
//   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
//   "pump_cmd");

// rclc_subscription_init_default(
//   &sub_fan_cmd, &node,
//   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
//   "fan_cmd");

//   // timers
//   rclc_timer_init_default(&t_soil, &support, RCL_MS_TO_NS(1000), cb_soil);
//   rclc_timer_init_default(&t_hb,   &support, RCL_MS_TO_NS(5000), cb_hb);

//   // executor
//   rclc_executor_init(&exec, &support.context, 4, &allocator);
//   rclc_executor_add_subscription(&exec, &sub_pump_cmd, &msg_pump, cb_pump, ON_NEW_DATA);
//   rclc_executor_add_subscription(&exec, &sub_fan_cmd,  &msg_fan,  cb_fan,  ON_NEW_DATA);
//   rclc_executor_add_timer(&exec, &t_soil);
//   rclc_executor_add_timer(&exec, &t_hb);
// }

// void setup() {
//   // 하드웨어 초기화
//   pinMode(FAN_PIN, OUTPUT); fan_off();      // fan.cpp와 일치(핀/동작) :contentReference[oaicite:1]{index=1}
//   pump_init();                               // water_pump.cpp 초기화 사용 :contentReference[oaicite:2]{index=2}

//   wifi_connect();
//   ota_begin();

//   while (!agent_alive()) { ArduinoOTA.handle(); delay(300); }
//   ros_begin();
// }

// void loop() {
//   ArduinoOTA.handle();

//   if (!agent_alive()) {
//     ros_teardown();
//     while (!agent_alive()) { ArduinoOTA.handle(); delay(300); }
//     ros_begin();
//   }

//   rclc_executor_spin_some(&exec, RCL_MS_TO_NS(5));
//   delay(5);
// }


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "keys.h"
#include "config.h"


WiFiUDP logUdp;

void logLine(const String& msg) {
  // 1) 있으면 USB 시리얼에도 찍고
  Serial.println(msg);

  // 2) UDP로도 전송
  logUdp.beginPacket(LOG_SERVER_IP, LOG_SERVER_PORT);

  const char* cmsg = msg.c_str();
  logUdp.write((const uint8_t*)cmsg, msg.length());
  logUdp.write('\n');

  logUdp.endPacket();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  logLine("");
  logLine("Booting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pwd);

  logLine("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logLine(".");
  }
  logLine("");
  logLine("WiFi connected, IP: ");
  logLine(WiFi.localIP().toString());

  logUdp.begin(0);   // 송신만 할 거면 0 써도 됨


  // OTA 기본 설정
  ArduinoOTA.setHostname("midshelf"); // 여기 이름은 마음대로
  // 필요하면 비번도:
  // ArduinoOTA.setPassword("1234");

  ArduinoOTA.onStart([]() {
    logLine("Start OTA");
  });
  ArduinoOTA.onEnd([]() {
    logLine("\nEnd OTA");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    logLine("Error[" + String((uint32_t)error) + "]");
  });

  ArduinoOTA.begin();
  logLine("OTA ready!.");
}

void loop() {
  ArduinoOTA.handle();   // OTA 업로드 받는 부분
  delay(10);             // 너무 꽉 잡아먹지 않게 살짝 쉬어주기
}
