import serial
import csv
from datetime import datetime

PORT = "COM5"        # ganti sesuai port Arduino
BAUD = 115200
OUTPUT_FILE = "plasdata_raw.csv"

ser = serial.Serial(PORT, BAUD, timeout=1)

with open(OUTPUT_FILE, "a", newline="") as file:
    writer = csv.writer(file)

    print("Mulai menyimpan data dari Arduino...")
    print("Tekan Ctrl+C untuk berhenti.")

    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()

            if not line:
                continue

            # Lewati teks informasi, hanya simpan baris CSV
            if line.startswith("sample_id") or line.startswith("S"):
                row = line.split(",")
                writer.writerow(row)
                file.flush()
                print(row)

    except KeyboardInterrupt:
        print("Penyimpanan dihentikan.")

ser.close()