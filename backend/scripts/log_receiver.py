import socket
from datetime import datetime

LOG_PORT = 9000      # 위에서 LOG_SERVER_PORT랑 맞춰야 함
LOG_FILE = "smartfarm.log"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", LOG_PORT))

print(f"Listening for logs on UDP port {LOG_PORT}...")

with open(LOG_FILE, "a", encoding="utf-8") as f:
    while True:
        data, addr = sock.recvfrom(4096)
        msg = data.decode(errors="ignore").strip()
        ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        line = f"[{ts}] {addr[0]}: {msg}"
        print(line)
        f.write(line + "\n")
        f.flush()
