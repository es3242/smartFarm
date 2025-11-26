from fastapi import FastAPI, Query
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel            
from .db import get_connection, query_latest
from .config import SHELVES, topic_for  
import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool

app = FastAPI(title="Smartfarm API")
class FanControlNode(Node):
    def __init__(self):
        super().__init__("fan_control_node")
        # 선반별 fan_cmd 퍼블리셔 dict
        self.pub_fan_cmd = {}
        for shelf in SHELVES:
            topic = topic_for(shelf, "fan_cmd")
            self.pub_fan_cmd[shelf] = self.create_publisher(Bool, topic, 10)

    def set_manual_on(self, shelf: str):
        msg = Bool()
        msg.data = True
        self.pub_fan_cmd[shelf].publish(msg)
        self.get_logger().info(f"[{shelf}] FAN MANUAL ON")

    def set_auto(self, shelf: str):
        msg = Bool()
        msg.data = False
        self.pub_fan_cmd[shelf].publish(msg)
        self.get_logger().info(f"[{shelf}] FAN BACK TO AUTO")

# CORS 허용 (프론트엔드 개발 편하게)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# ROS init & node 준비 (FastAPI 프로세스 안에서 사용)
rclpy.init(args=None)
fan_node = FanControlNode()

class FanModeCmd(BaseModel):
    mode: str   # "manual_on" or "auto"


@app.post("/api/fan/{shelf}/mode")
def set_fan_mode(shelf: str, cmd: FanModeCmd):
    if shelf not in SHELVES:
        return {"error": "invalid shelf"}

    if cmd.mode == "manual_on":
        fan_node.set_manual_on(shelf)
    elif cmd.mode == "auto":
        fan_node.set_auto(shelf)
    else:
        return {"error": "invalid mode"}

    return {"ok": True, "shelf": shelf, "mode": cmd.mode}


@app.get("/api/shelves")
def list_shelves():
    return {"shelves": SHELVES}


@app.get("/api/samples/{shelf}")
def get_samples(
    shelf: str,
    limit: int = Query(100, ge=1, le=500),
):
    if shelf not in SHELVES:
        return {"error": "invalid shelf"}

    conn = get_connection()
    rows = query_latest(conn, shelf, limit=limit)
    # rows: [(ts, temp, hum, soil_pct, soil_raw, pump_state, fan_state), ...]
    data = [
        {
            "ts": r[0],
            "temp": r[1],
            "hum": r[2],
            "soil_pct": r[3],
            "soil_raw": r[4],
            "pump_state": r[5],
            "fan_state": r[6],
        }
        for r in rows
    ]
    return {"shelf": shelf, "data": data}
