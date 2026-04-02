import socket, time, sys

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = 54000
AIRCRAFT_ID = "AC001"

with open("data/sample/testing.txt") as f:
    lines = f.readlines()

with socket.create_connection((HOST, PORT)) as s:
    for line in lines:
        parts = line.strip().split(',')
        if len(parts) < 2:
            continue
        timestamp, fuel = parts[0].strip(), parts[1].strip()
        packet = f"{AIRCRAFT_ID},{timestamp},{fuel}\n"
        s.sendall(packet.encode())
        time.sleep(0.05)  
    print("Done sending")