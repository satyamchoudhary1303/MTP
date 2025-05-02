import socket
import csv
import time

UDP_IP = "0.0.0.0"
UDP_PORT = 12345
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

with open("data/wifi_angle_log.csv", "w", newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["Timestamp (s)", "Angle (deg)"])
    start = time.time()
    print(f"[Listening] on UDP {UDP_IP}:{UDP_PORT}")

    try:
        while True:
            data, addr = sock.recvfrom(1024)
            line = data.decode().strip()
            if "Angle:" in line:
                angle = float(line.split(":")[1].strip())
                timestamp = time.time() - start
                writer.writerow([round(timestamp, 5), angle])
                print(f"{timestamp:.2f}s -> {angle:.2f}° from {addr[0]}")
    except KeyboardInterrupt:
        print("\n[Stopped] Logging.")
