import sqlite3
from pathlib import Path
from .config import DB_PATH

SCHEMA = """
CREATE TABLE IF NOT EXISTS samples (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    shelf TEXT NOT NULL,
    ts   DATETIME DEFAULT CURRENT_TIMESTAMP,

    temp REAL,
    hum  REAL,
    soil_pct REAL,
    soil_raw INTEGER,
    pump_state INTEGER,
    fan_state  INTEGER
);
"""

def get_connection() -> sqlite3.Connection:
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    conn.execute("PRAGMA journal_mode=WAL;")
    conn.execute(SCHEMA)
    return conn

def insert_sample(conn, shelf, temp, hum, soil_pct, soil_raw,
                  pump_state, fan_state):
    conn.execute(
        """
        INSERT INTO samples
        (shelf, temp, hum, soil_pct, soil_raw, pump_state, fan_state)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        """,
        (shelf, temp, hum, soil_pct, soil_raw,
         int(pump_state) if pump_state is not None else None,
         int(fan_state) if fan_state is not None else None)
    )
    conn.commit()

def query_latest(conn, shelf: str, limit: int = 100):
    cur = conn.execute(
        """
        SELECT ts, temp, hum, soil_pct, soil_raw, pump_state, fan_state
        FROM samples
        WHERE shelf = ?
        ORDER BY id DESC
        LIMIT ?
        """,
        (shelf, limit)
    )
    rows = cur.fetchall()
    # 최신이 위에 있으니, 그래프 편하게 쓰려면 뒤집어서 반환
    rows.reverse()
    return rows
