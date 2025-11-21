from pathlib import Path

# DB 파일 위치
BASE_DIR = Path(__file__).resolve().parent.parent
DB_PATH = BASE_DIR / "db" / "farm.db"

# 로깅 주기 (초) - 10초마다 스냅샷 저장
LOG_INTERVAL_SEC = 10.0

# ROS 네임스페이스 / 토픽들
SHELVES = ["bottom", "mid", "top"]

# 예: /smartfarm/mid/soil_moisture 이런 식으로 들어온다고 가정
def topic_for(shelf: str, name: str) -> str:
    return f"/smartfarm/{shelf}/{name}"
