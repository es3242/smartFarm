#!/bin/bash
set -e

# ROS 환경
source /opt/ros/humble/setup.bash

cd /workspace

# DB 폴더 없으면 생성
mkdir -p db

# 1) micro-ROS agent (ESP32에서 오는 UDP를 ROS 토픽으로)
micro-ros-agent udp4 --port 8888 -v6 &
AGENT_PID=$!

# 2) 센서 로그 → DB 로거
python3 -m app.logger_node &
LOGGER_PID=$!

# 3) FastAPI 서버 (웹/대시보드 API)
uvicorn app.api:app --host 0.0.0.0 --port 8000 &
API_PID=$!

# 하나라도 죽으면 컨테이너 종료
wait -n $AGENT_PID $LOGGER_PID $API_PID
EXIT_CODE=$?

echo "One of the services exited with code $EXIT_CODE. Shutting down..."
kill $AGENT_PID $LOGGER_PID $API_PID 2>/dev/null || true
exit $EXIT_CODE
