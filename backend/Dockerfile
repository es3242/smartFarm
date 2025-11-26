# ROS Humble + Python3.10 포함 이미지
FROM ros:humble-ros-base

# 기본 유틸 + micro-ROS agent + pip
RUN apt-get update && apt-get install -y \
      python3-pip \
      && rm -rf /var/lib/apt/lists/*

# 작업 디렉토리
WORKDIR /workspace

# 백엔드 코드 복사
COPY app ./app
COPY db ./db
COPY web ./web
COPY requirements.txt ./requirements.txt

# Python 라이브러리 설치 (rclpy는 ROS에서 제공)
RUN pip install --no-cache-dir -r requirements.txt

# 엔트리포인트 스크립트 복사
COPY entrypoint.sh ./entrypoint.sh
RUN chmod +x ./entrypoint.sh

# FastAPI 포트
EXPOSE 8000

CMD ["./entrypoint.sh"]
