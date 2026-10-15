import socket
import json

UDP_PORT = 4210

print(f"==================================================")
print(f" MENUNGGU DATA UDP ESP32 (PORT {UDP_PORT})")
print(f"==================================================\n")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", UDP_PORT))

try:
    while True:
        data, addr = sock.recvfrom(4096)
        pesan = data.decode('utf-8', errors='ignore')
        
        try:
            # Ubah teks JSON menjadi objek Python
            data_json = json.loads(pesan)
            
            # Ambil identitas device
            device_id = data_json.get("device_id", "Unknown Node")
            
            print(f"\n[{addr[0]}] --- {device_id} ---")
            
            # Print setiap data dengan format rapi (Key: Value)
            for key, value in data_json.items():
                # Abaikan device_id karena sudah diprint sebagai judul
                if key != "device_id":
                    print(f"  {key:<18}: {value}")
                    
            print("-" * 40)
            
        except json.JSONDecodeError:
            # Jika bukan format JSON, print biasa
            print(f"\n[{addr[0]}] Teks biasa: {pesan}")
            
except KeyboardInterrupt:
    print("\nBerhenti mendengarkan UDP.")
finally:
    sock.close()
