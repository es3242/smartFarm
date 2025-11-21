import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSHistoryPolicy

from std_msgs.msg import Float32, Bool
from .config import SHELVES, topic_for, LOG_INTERVAL_SEC
from .db import get_connection, insert_sample


class ShelfState:
    def __init__(self):
        self.temp = None
        self.hum = None
        self.soil_pct = None
        self.soil_raw = None
        self.pump_state = None
        self.fan_state = None


class SmartfarmLogger(Node):
    def __init__(self):
        super().__init__("smartfarm_logger")

        self.conn = get_connection()

        # 선반별 상태 dict
        self.state = {shelf: ShelfState() for shelf in SHELVES}

        sensor_qos = QoSProfile(
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=10,
        )


        # 구독 설정
        for shelf in SHELVES:
            # 토양
            self.create_subscription(
                Float32,
                topic_for(shelf, "soil_moisture"),
                lambda msg, s=shelf: self.cb_soil(s, msg),
                sensor_qos,   # ← 10 대신 sensor_qos
            )

            # 온도/습도도 나중에 ESP32에서 BEST_EFFORT로 퍼블리시할 거면 같이 sensor_qos
            self.create_subscription(
                Float32,
                topic_for(shelf, "temperature"),
                lambda msg, s=shelf: self.cb_temp(s, msg),
                sensor_qos,
            )
            self.create_subscription(
                Float32,
                topic_for(shelf, "humidity"),
                lambda msg, s=shelf: self.cb_hum(s, msg),
                sensor_qos,
            )

            # 펌프/팬은 ESP 쪽이 reliable(default)라면 그냥 10 유지해도 되고,
            # 통일하고 싶으면 sensor_qos 써도 됨 (둘 다 BEST_EFFORT로 맞추기)
            self.create_subscription(
                Bool,
                topic_for(shelf, "pump_state"),
                lambda msg, s=shelf: self.cb_pump(s, msg),
                10,
            )
            self.create_subscription(
                Bool,
                topic_for(shelf, "fan_state"),
                lambda msg, s=shelf: self.cb_fan(s, msg),
                10,
            )


        # 로깅 타이머
        self.create_timer(LOG_INTERVAL_SEC, self.log_snapshot)
        self.get_logger().info("SmartfarmLogger started")

    # 콜백들
    def cb_soil(self, shelf, msg: Float32):
        self.state[shelf].soil_pct = float(msg.data)
        # soil_raw는 나중에 별도 토픽 쓰거나, ESP에서 같이 퍼블하고 싶으면 동일 패턴으로 추가

    def cb_temp(self, shelf, msg: Float32):
        self.state[shelf].temp = float(msg.data)

    def cb_hum(self, shelf, msg: Float32):
        self.state[shelf].hum = float(msg.data)

    def cb_pump(self, shelf, msg: Bool):
        self.state[shelf].pump_state = bool(msg.data)

    def cb_fan(self, shelf, msg: Bool):
        self.state[shelf].fan_state = bool(msg.data)

    def log_snapshot(self):
        for shelf, st in self.state.items():
            # 최소한 soil_pct만 있어도 저장
            if st.soil_pct is None:
                continue

            insert_sample(
                self.conn,
                shelf,
                st.temp,
                st.hum,
                st.soil_pct,
                st.soil_raw,
                st.pump_state,
                st.fan_state,
            )
        self.get_logger().debug("snapshot logged")


def main():
    rclpy.init()
    node = SmartfarmLogger()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("Interrupted by user")
    finally:
        node.destroy_node()
        try:
            rclpy.shutdown()
        except RCLError:
            # 이미 shutdown된 경우 여기로 들어옴 → 무시
            pass


if __name__ == "__main__":
    main()