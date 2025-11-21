#include <Arduino.h>
#include <WiFi.h>                  // set_microros_wifi_transports 내부에서 사용
#include <ArduinoOTA.h>            // 에러 루프에서 OTA 유지하려면

#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/float32.h>
#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/int32.h>

#include "keys.h"        // wifi_ssid, wifi_pwd
#include "config.h"      // HOSTNAME, NS, AGENT_PORT
#include "func_soil.h"   // get_soilraw(), rawToPercent()
#include "fan.h"         // fan_on(), fan_off(), FAN_PIN
#include "water_pump.h"  // pump_init(), pump_on(), pump_off()
#include "log.h"
#include "micro_ros_node.h"

// ===== macro & 에러 루프 =====
static void error_loop();

#define RCCHECK(fn)      { rcl_ret_t rc = fn; if (rc != RCL_RET_OK) { \
  logLine("[microROS] hard error rc=" + String((int)rc)); error_loop(); } }

#define RCSOFTCHECK(fn)  { rcl_ret_t rc = fn; if (rc != RCL_RET_OK) { \
  logLine("[microROS] soft error rc=" + String((int)rc)); } }

// ===== micro-ROS 전역 =====
static rclc_support_t   support;
static rcl_allocator_t  allocator;
static rcl_node_t       node;
static rclc_executor_t  executor;

// 퍼블리셔
static rcl_publisher_t  pub_soil;
static rcl_publisher_t  pub_pump_state;
static rcl_publisher_t  pub_fan_state;
static rcl_publisher_t  pub_hb;

// 서브스크립션
static rcl_subscription_t sub_pump_cmd;
static rcl_subscription_t sub_fan_cmd;

// 타이머
static rcl_timer_t timer_soil;
static rcl_timer_t timer_hb;
static rcl_timer_t timer_fan;

// 메시지 인스턴스
static std_msgs__msg__Float32 msg_soil;
static std_msgs__msg__Bool    msg_pump;
static std_msgs__msg__Bool    msg_fan;
static std_msgs__msg__Int32   msg_hb;

// 상태
static bool pump_on_flag = false;
static uint32_t hb_seq   = 0;

// ===== 내부 함수 선언 =====
static float read_soil_pct();
static void soil_timer_callback(rcl_timer_t* timer, int64_t last_call_time);
static void fan_timer_callback(rcl_timer_t* timer, int64_t last_call_time);
static void hb_timer_callback(rcl_timer_t* timer, int64_t last_call_time);
static void pump_cmd_callback(const void* msgin);
static void fan_cmd_callback(const void* msgin);

// ===== 구현 =====
static void error_loop() {
  while (true) {
    ArduinoOTA.handle();
    delay(100);
  }
}

static float read_soil_pct() {
  int raw = get_soilraw();
  int pct = rawToPercent(raw);
  return (float)pct;
}

static void soil_timer_callback(rcl_timer_t* timer, int64_t last_call_time) {
  (void)timer;
  (void)last_call_time;

  msg_soil.data = read_soil_pct();
  RCSOFTCHECK(rcl_publish(&pub_soil, &msg_soil, NULL));

  msg_pump.data = pump_on_flag;
  RCSOFTCHECK(rcl_publish(&pub_pump_state, &msg_pump, NULL));


}

static void fan_timer_callback(rcl_timer_t* timer, int64_t last_call_time)
{
    (void)timer;
    (void)last_call_time;
    fan_auto_update();

    msg_fan.data = fan_on_flag;
    RCSOFTCHECK(rcl_publish(&pub_fan_state, &msg_fan, NULL));
}

static void hb_timer_callback(rcl_timer_t* timer, int64_t last_call_time) {
  (void)timer;
  (void)last_call_time;

  msg_hb.data = (int32_t)hb_seq++;
  RCSOFTCHECK(rcl_publish(&pub_hb, &msg_hb, NULL));

  if (hb_seq % 5 == 0) {
    logLine("[microROS] heartbeat: " + String(msg_hb.data));
  }
}

static void pump_cmd_callback(const void* msgin) {
  auto* m = (const std_msgs__msg__Bool*)msgin;
  pump_on_flag = m->data;

  if (pump_on_flag) pump_on();
  else              pump_off();

  msg_pump.data = pump_on_flag;
  RCSOFTCHECK(rcl_publish(&pub_pump_state, &msg_pump, NULL));
}

static void fan_cmd_callback(const void* msgin) {
  auto* m = (const std_msgs__msg__Bool*)msgin;

  if (m->data) {
    // ✅ 수동 강제 ON
    fan_mode     = FAN_MODE_MANUAL;
    fan_hw_state = true;
    fan_on_flag  = true;
    fan_on();
    logLine("FAN MANUAL ON");
  } else {
    // ✅ 자동 모드 복귀 (이제는 '시계 기반 AUTO')
    fan_mode = FAN_MODE_AUTO;

    // 현재 시각(덴버) 기준으로 바로 상태 맞추기
    fan_auto_update();
    logLine("FAN BACK TO AUTO");
  }

  // 현재 상태 퍼블리시
  msg_fan.data = fan_on_flag;
  RCSOFTCHECK(rcl_publish(&pub_fan_state, &msg_fan, NULL));
}



// ===== 외부 노출 함수 =====
void microRosInit() {
  logLine("[microROS] init start");

  // Agent IP (Ubuntu VM IP)
  IPAddress agent_ip(192, 168, 1, 46);

  set_microros_wifi_transports(
      (char*)wifi_ssid,
      (char*)wifi_pwd,
      agent_ip,
      AGENT_PORT);

  allocator = rcl_get_default_allocator();

  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  RCCHECK(rclc_node_init_default(
      &node,
      HOSTNAME,
      NS,
      &support));

  // 퍼블리셔
  RCCHECK(rclc_publisher_init_best_effort(
      &pub_soil,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
      "soil_moisture"));

  RCCHECK(rclc_publisher_init_default(
      &pub_pump_state,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
      "pump_state"));

  RCCHECK(rclc_publisher_init_default(
      &pub_fan_state,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
      "fan_state"));

  RCCHECK(rclc_publisher_init_best_effort(
      &pub_hb,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "heartbeat"));

  // 서브스크립션
  RCCHECK(rclc_subscription_init_default(
      &sub_pump_cmd,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
      "pump_cmd"));

  RCCHECK(rclc_subscription_init_default(
      &sub_fan_cmd,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
      "fan_cmd"));

  // 타이머
  RCCHECK(rclc_timer_init_default(
      &timer_soil,
      &support,
      RCL_MS_TO_NS(1000),
      soil_timer_callback));

  RCCHECK(rclc_timer_init_default(
      &timer_hb,
      &support,
      RCL_MS_TO_NS(5000),
      hb_timer_callback));

  RCCHECK(rclc_timer_init_default(
    &timer_fan,
    &support,
    RCL_MS_TO_NS(1000),    // 1초 간격 추천
    fan_timer_callback));


  // executor
  RCCHECK(rclc_executor_init(
      &executor,
      &support.context,
      4,
      &allocator));

  RCCHECK(rclc_executor_add_subscription(
      &executor,
      &sub_pump_cmd,
      &msg_pump,
      pump_cmd_callback,
      ON_NEW_DATA));

  RCCHECK(rclc_executor_add_subscription(
      &executor,
      &sub_fan_cmd,
      &msg_fan,
      fan_cmd_callback,  
      ON_NEW_DATA));

  RCCHECK(rclc_executor_add_timer(&executor, &timer_soil));
  RCCHECK(rclc_executor_add_timer(&executor, &timer_hb));
  RCCHECK(rclc_executor_add_timer(&executor, &timer_fan));  

  hb_seq = 0;
  pump_on_flag = false;
  fan_on_flag  = false;

  logLine("[microROS] init done");
}

void microRosSpinOnce() {
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
}
