import socket

HOST = '0.0.0.0'   # listen on all interfaces
PORT = 5001        # ESP connects here for debug logs

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((HOST, PORT))
server.listen(1)

print(f"[DBG] Waiting for ESP on port {PORT}...")

conn, addr = server.accept()
print(f"[DBG] ESP connected from {addr}")

buf = ""
while True:
    chunk = conn.recv(1024).decode(errors='replace')
    if not chunk:
        print("[DBG] ESP disconnected.")
        break
    buf += chunk
    while '\n' in buf:
        line, buf = buf.split('\n', 1)
        print(f"[ESP] {line.rstrip()}")