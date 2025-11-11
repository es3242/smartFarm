#include <WiFi.h>
#include <ArduinoOTA.h>
#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

#define WIFI_SSID   
#define WIFI_PASS   
#define AGENT_IP    
#define AGENT_PORT  8888
#define HOSTNAME    "shelf-mid"   // 다른 보드는 "shelf-mid" 등

rclc_support_t support; rcl_allocator_t allocator;
rcl_node_t node; rcl_publisher_t pub; std_msgs__msg__Int32 msg;
rcl_timer_t timer; rclc_executor_t exec;

void wifi_connect(){
  WiFi.mode(WIFI_STA); WiFi.setHostname(HOSTNAME);
   WiFi.setSleep(false); 
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status()!=WL_CONNECTED) delay(100);
}
void ota_begin(){ ArduinoOTA.setHostname(HOSTNAME);  ArduinoOTA.setPort(3232);
 ArduinoOTA.begin(); }
void timer_cb(rcl_timer_t*, int64_t){ static int32_t c=0; msg.data=c++; rcl_publish(&pub,&msg,nullptr); }

void microros_begin(){
  set_microros_wifi_transports(WIFI_SSID, WIFI_PASS, AGENT_IP, AGENT_PORT);
  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, HOSTNAME, "", &support);
  rclc_publisher_init_default(&pub, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "counter_" HOSTNAME);
  rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(1000), timer_cb);
  rclc_executor_init(&exec, &support.context, 1, &allocator);
  rclc_executor_add_timer(&exec, &timer);
}

void setup(){ wifi_connect(); ota_begin(); microros_begin(); }
void loop(){ ArduinoOTA.handle(); rclc_executor_spin_some(&exec, RCL_MS_TO_NS(5)); delay(5); }
