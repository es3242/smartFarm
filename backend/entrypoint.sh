#!/bin/bash
set -e

# ROS 환경 세팅
source /opt/ros/humble/setup.bash

cd /workspace

# DB 폴더 없으면 생성
mkdir -p db

echo "[entrypoint] starting micro-ROS agent..."
# micro-ROS Agent (ESP32에서 오는 UDP → ROS2)
ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888 -v6 &
AGENT_PID=$!

echo "[entrypoint] starting logger_node..."
# rclpy logger (토픽 → SQLite)
python3 -m app.logger_node &
LOGGER_PID=$!

echo "[entrypoint] starting FastAPI..."
# 웹 API (ROS 제어 + DB 제공)
uvicorn app.api:app --host 0.0.0.0 --port 8000 &
API_PID=$!

# 하나라도 죽으면 컨테이너 종료
wait -n $AGENT_PID $LOGGER_PID $API_PID
EXIT_CODE=$?

echo "[entrypoint] one of services exited with code $EXIT_CODE, shutting down..."
kill $AGENT_PID $LOGGER_PID $API_PID 2>/dev/null || true
exit $EXIT_CODE
