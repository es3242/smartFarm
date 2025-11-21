from fastapi import FastAPI, Query
from fastapi.middleware.cors import CORSMiddleware
from .db import get_connection, query_latest
from .config import SHELVES

app = FastAPI(title="Smartfarm API")

# CORS 허용 (프론트엔드 개발 편하게)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


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
