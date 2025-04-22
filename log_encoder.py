import serial
import csv
import time

# Replace COM3 with your Pico's COM port
ser = serial.Serial('COM6', 115200)
time.sleep(2)  # wait for serial connection to initialize

with open("angle_log.csv", "w", newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["Timestamp (s)", "Angle (deg)"])
    
    start_time = time.time()

    try:
        while True:
            line = ser.readline().decode().strip()
            if "Angle:" in line:
                angle = float(line.split(":")[1].split()[0])
                timestamp = time.time() - start_time
                writer.writerow([round(timestamp, 5), angle])
                print(f"{timestamp:.4f}s -> {angle}°")
    except KeyboardInterrupt:
        print("Logging stopped.")
