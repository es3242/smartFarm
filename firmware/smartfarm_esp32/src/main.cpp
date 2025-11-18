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
// ====== micro-ROS 추가 ======

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "keys.h"
#include "config.h"

// ====== micro-ROS ======
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>

// 에러 체크 매크로
#define RCCHECK(fn) { rcl_ret_t rc = fn; if (rc != RCL_RET_OK) { logLine("[microROS] hard error rc=" + String((int)rc)); error_loop(); } }
#define RCSOFTCHECK(fn) { rcl_ret_t rc = fn; if (rc != RCL_RET_OK) { logLine("[microROS] soft error rc=" + String((int)rc)); } }

#define AGENT_IP    
#define AGENT_PORT  8888
#define HOSTNAME    "midshelf"
#define NS          "smartfarm/mid"

WiFiUDP logUdp;
bool loggingReady = false;


// ====== micro-ROS 전역 ======
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rclc_executor_t executor;

rcl_publisher_t pub_hb;
rcl_timer_t timer_hb;
std_msgs__msg__Int32 msg_hb;
uint32_t hb_seq = 0;

void logLine(const String& msg) {
  Serial.println(msg);   // 시리얼은 항상 찍어도 됨

  if (loggingReady && WiFi.status() == WL_CONNECTED) {
    logUdp.beginPacket(LOG_SERVER_IP, LOG_SERVER_PORT);
    const char* cmsg = msg.c_str();
    logUdp.write((const uint8_t*)cmsg, msg.length());
    logUdp.write('\n');
    logUdp.endPacket();
  }
}


// ====== micro-ROS heartbeat 콜백 ======
void hb_timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  (void)timer;
  (void)last_call_time;

  msg_hb.data = (int32_t)hb_seq++;
  rcl_ret_t rc = rcl_publish(&pub_hb, &msg_hb, NULL);

  if (rc == RCL_RET_OK) {
    if (hb_seq % 5 == 0) {  // 너무 많이 안 찍게 5번에 한 번만
      logLine("[microROS] heartbeat: " + String(msg_hb.data));
    }
  } else {
    logLine("[microROS] publish error: " + String((int)rc));
  }
}

// ====== micro-ROS 최소 초기화 ======
void microRosInit() {
  logLine("[microROS] init start");

  // WiFi는 이미 붙어 있으니까 UDP transport만 설정

  IPAddress agent_ip(192, 168, 1, 46);   // <- 네 AGENT_IP 숫자로 맞춰서
  set_microros_wifi_transports(
      (char*)wifi_ssid,                  // const char* → char* 캐스팅
      (char*)wifi_pwd,
      agent_ip,
      AGENT_PORT
  );

  allocator = rcl_get_default_allocator();

  rcl_ret_t rc = rclc_support_init(&support, 0, NULL, &allocator);
  if (rc != RCL_RET_OK) {
    logLine("[microROS] support_init failed: " + String((int)rc));
    return;
  }

  rc = rclc_node_init_default(
        &node,
        HOSTNAME,   // 노드 이름
        NS,         // 네임스페이스
        &support);
  if (rc != RCL_RET_OK) {
    logLine("[microROS] node_init failed: " + String((int)rc));
    return;
  }

  // /smartfarm/mid/heartbeat (Int32, best effort)
  rc = rclc_publisher_init_best_effort(
        &pub_hb,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "heartbeat");
  if (rc != RCL_RET_OK) {
    logLine("[microROS] pub_init failed: " + String((int)rc));
    return;
  }

  // 1초마다 타이머
  rc = rclc_timer_init_default(
        &timer_hb,
        &support,
        RCL_MS_TO_NS(1000),
        hb_timer_callback);
  if (rc != RCL_RET_OK) {
    logLine("[microROS] timer_init failed: " + String((int)rc));
    return;
  }

  rc = rclc_executor_init(&executor, &support.context, 1, &allocator);
  if (rc != RCL_RET_OK) {
    logLine("[microROS] executor_init failed: " + String((int)rc));
    return;
  }

  rc = rclc_executor_add_timer(&executor, &timer_hb);
  if (rc != RCL_RET_OK) {
    logLine("[microROS] add_timer failed: " + String((int)rc));
    return;
  }

  logLine("[microROS] init done");
}












void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Booting...");   // ← 여기서는 굳이 logLine 안 써도 됨

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pwd);

  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // WiFi 붙은 후에만 UDP 시작
  logUdp.begin(0);           // 🔹 여기서 UDP 소켓 열고
  loggingReady = true;       // 🔹 이제부터 logLine이 UDP 사용 가능

  logLine("WiFi connected, IP: " + WiFi.localIP().toString());

  ArduinoOTA.setHostname("midshelf");

  ArduinoOTA.onStart([]() {
    logLine("Start OTA");
  });
  ArduinoOTA.onEnd([]() {
    logLine("End OTA");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    logLine("Error[" + String((uint32_t)error) + "]");
  });

  ArduinoOTA.begin();
  logLine("OTA ready!~~~.");
   // --- micro-ROS 시작 ---
  microRosInit();
}

void loop() {
  ArduinoOTA.handle();
    // micro-ROS 실행 (콜백/타이머)
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));

  delay(10);
}
