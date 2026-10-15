# 🌱 NURTURE: Sistem Otomatisasi Pertanian Cerdas Mandiri Energi dengan Pemantauan Berbasis Web

[![Platform: ESP32-S3](https://img.shields.io/badge/Platform-ESP32--S3-orange.svg)](https://www.espressif.com/)
[![Framework: Arduino / C++](https://img.shields.io/badge/Framework-Arduino%20%2F%20C%2B%2B-blue.svg)](https://www.arduino.cc/)
[![Cloud: ThingSpeak IoT](https://img.shields.io/badge/IoT%20Cloud-ThingSpeak-brightgreen.svg)](https://thingspeak.com/)
[![Hardware: KiCad EDA](https://img.shields.io/badge/Hardware-KiCad%20EDA-314CB0.svg)](https://www.kicad.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> **Program Pengabdian kepada Masyarakat (PKM) 2026**  
> **Universitas Gadjah Mada**  
> Lokasi Implementasi: Kalurahan Margorejo, Kapanewon Tempel, Kabupaten Sleman, D.I. Yogyakarta  
> **Dosen Pembimbing:** Nur Achmad Sulistyo Putro, S.Si., M.Cs., Ph.D.

---

## 📌 1. Latar Belakang & Deskripsi Proyek

Pertanian presisi (*precision agriculture*) membutuhkan data akurat mengenai ketersediaan nutrisi tanah dan kondisi iklim mikro secara *real-time*. Di persawahan konvensional, penentuan dosis pupuk dan jadwal pengairan kerap hanya mengandalkan perkiraan, yang berisiko menyebabkan degradasi unsur hara, pemborosan air, dan pembengkakan biaya produksi.

**NURTURE** (*Nutrient & Resource Tracking for Unified Resilient Ecology*) adalah sistem pemantauan dan otomasi pertanian presisi berbasis **Internet of Things (IoT)** yang beroperasi secara mandiri (*off-grid*) menggunakan **Pembangkit Listrik Tenaga Surya (PLTS)**. Sistem ini memiliki toleransi tinggi terhadap gangguan jaringan (*offline-tolerant*), perlindungan gagal-aman (*fail-safe*), dan dilengkapi dengan arsitektur **PCB Bertingkat (Dual-Tier Stack PCB)** untuk menjamin kestabilan operasional jangka panjang.

---

## 🏗️ 2. Arsitektur Perangkat Keras (Hardware)

Sistem mengimplementasikan pemisahan fisik antara sirkuit daya berarus tinggi dan sirkuit pengendali logika mikro melalui **Desain PCB Bertingkat (Stack Architecture)**:

```
┌─────────────────────────────────────────────────────────────┐
│                 PCB TINGKAT ATAS (UPPER TIER)               │
│  • ESP32-S3 DevKitC-1 N16R8V (Dual-Core 240MHz, Wi-Fi/BLE) │
│  • Modul Transceiver RS485 MAX485 (Modbus RTU Master)       │
│  • Driver Transistor NPN (Q1, Q2, Q3) & Resistor 2kΩ        │
│  • Terminal Antarmuka Sensor (NPK, DHT22, LDR, Soil Analog) │
│  • Terminal Header Relay Aktuator (Fan, Valve, UV Lamp)     │
└──────────────────────────────┬──────────────────────────────┘
                               │ Interkoneksi Daya Vertikal
                               │ (J_12V_FROM_BOTTOM1 & J_5v_3v3)
┌──────────────────────────────┴──────────────────────────────┐
│                PCB TINGKAT BAWAH (LOWER TIER)               │
│  • Masukan Daya 12V DC dari Baterai VRLA / SCC PLTS         │
│  • Rel 12V DC Langsung (Solenoid Valve, Kipas, Lampu UV)    │
│  • Modul Step-Down 12V -> 5V (Kapasitas 3A untuk Logika)    │
│  • Modul Step-Down 12V -> 3.3V (Regulasi Logika Sensitif)   │
│  • Common Ground Bus (Pencegah Ground Loop & Noise Motorik) │
└─────────────────────────────────────────────────────────────┘
```

### ⚡ Sistem Catu Daya PLTS (Mandiri Energi 24 Jam)
* **Panel Surya & Solar Charge Controller (SCC):** Sumber pengisian baterai tenaga surya mandiri.
* **Baterai VRLA 12V 12Ah:** Mampu menyuplai daya sistem > 3 hari berturut-turut tanpa radiasi matahari.
* **Dual Sensor Daya INA226 (I2C):**
  * Modul 1 (`0x44`): Memantau arus & daya pengisian dari SCC ke Baterai (*Charging Current*).
  * Modul 2 (`0x45`): Memantau arus & konsumsi daya aktual sistem (*Load Current*).

### 📡 Sensor yang Terintegrasi
1. **Sensor Tanah NPK 7-in-1 (Modbus RS485 Industrial):** Mengukur Nitrogen (N), Fosfor (P), Kalium (K), pH tanah, Electrical Conductivity (EC / Salinitas), suhu tanah, dan kelembapan tanah.
2. **Sensor Kelembapan Tanah Analog Kapasitif:** Probe analog pendukung kalibrasi tanah lokal.
3. **Dual Sensor DHT22 Luar:** Mengukur suhu dan kelembapan udara lingkungan sekitar (dirata-ratakan).
4. **Sensor DHT22 Dalam:** Memantau suhu dan kelembapan di dalam kotak panel kontrol.
5. **Sensor Cahaya LDR:** Memantau intensitas radiasi cahaya matahari.
6. **Real-Time Clock (RTC DS3231):** Menjaga presisi waktu operasional secara luring (*offline*).

---

## ⚙️ 3. Fitur Utama & Logika Otomasi

Sistem mengambil keputusan otomasi secara mandiri setiap 1 detik di tingkat mikrokontroler:

| Fitur | Aturan Operasional | Pengaman (*Fail-Safe*) |
|---|---|---|
| **Penyiraman Cerdas (*Solenoid Valve*)** | Buka otomatis pukul **07:00 – 07:10 WIB** (Pagi) dan pukul **17:20 – 17:30 WIB** (Sore) selama 10 menit. | Batas maksimum 15 menit otomatis tutup paksa untuk mencegah banjir / pompa jebol. |
| **Pendingin Panel (*Cooling Fan*)** | Menyala otomatis saat suhu dalam boks $\ge 34.0^\circ\text{C}$ selama 5 menit per siklus. | Siklus otomatis di-reset; menyala kembali jika boks masih panas. |
| **Perangkap Hama (*UV Trap*)** | Menyala otomatis malam hari pukul **18:00 – 06:00 WIB** via modul RTC DS3231. | Bekerja mandiri tanpa ketergantungan koneksi internet. |
| **Monitoring Nutrisi & EC** | Pembacaan Modbus RTU setiap 1 detik untuk N, P, K, pH, dan EC (µS/cm). | Validasi frame checksum untuk mengeliminasi pembacaan *glitch*. |

---

## 🌐 4. Transmisi Data & Monitoring IoT

Sistem menggunakan arsitektur pemantauan ganda:

1. **Telemetri Lokal Real-Time (UDP Broadcast):**
   * ESP32 memancarkan data mentah seluruh sensor setiap **1 detik** ke port `4210` di jaringan lokal Wi-Fi tanpa memerlukan kuota internet.
   * Sangat berguna untuk kalibrasi lapangan dan inspeksi langsung teknisi via laptop.
2. **Cloud IoT Dashboard (ThingSpeak HTTPS):**
   * Data diunggah setiap **16 detik sekali** (mendekati batas *rate limit* platform ThingSpeak).
   * Data dibagi menjadi 2 channel berurutan dengan jeda 5 detik untuk menjaga kapasitas memori (heap) ESP32:
     * **Channel Utama:** Data NPK (N, P, K, pH, EC), kelembapan tanah rata-rata, suhu & kelembapan lingkungan luar.
     * **Channel Pendukung:** Suhu tanah NPK, voltase & arus baterai (INA226), voltase & arus beban (INA226), suhu kotak panel, daya sistem aktual (Watt), dan status aktuator (*Bitmask Integer 0–15*).

---

## 📁 5. Struktur Direktori Repositori

```text
├── Buku_Panduan_Wiring_NURTURE_Margorejo.docx  # Buku Panduan Teknis & Wiring lengkap (Word)
├── KODE_ESP_NODE1_SEMOGASUDAHFIX/              # Firmware Arduino IDE untuk ESP32 Node 1
│   └── KODE_ESP_NODE1_SEMOGASUDAHFIX.ino       # Kode program Node 1 (AGRI-01)
├── KODE_ESP_NODE2_SEMOGASUDAHFIX/              # Firmware Arduino IDE untuk ESP32 Node 2
│   └── KODE_ESP_NODE2_SEMOGASUDAHFIX.ino       # Kode program Node 2 (AGRI-02)
├── cek_udp.py                                  # Script Python monitoring telemetri UDP lokal
├── generate_manual.py                          # Script generator dokumen Word buku panduan
├── docx_helpers.py                             # Modul helper pembuat format tabel docx
├── platformio.ini                              # Konfigurasi PlatformIO
├── save_data.py                                # Script pengarsipan data telemetri
└── README.md                                   # Dokumentasi utama proyek
```

---

## 🚀 6. Panduan Penggunaan & Pengujian

### A. Pengunggahan Firmware ke ESP32-S3
1. Buka folder `KODE_ESP_NODE1_SEMOGASUDAHFIX/` atau `KODE_ESP_NODE2_SEMOGASUDAHFIX/` di **Arduino IDE**.
2. Pasang board profile: **ESP32 by Espressif Systems** (pilih board: `ESP32S3 Dev Module`).
3. Konfigurasi board di Arduino IDE:
   * **USB CDC On Boot:** `Enabled`
   * **Flash Size:** `16MB (128Mb)`
   * **Partition Scheme:** `16M Flash (3MB APP/9.9MB FATFS)`
   * **PSRAM:** `OPI PSRAM`
4. Pastikan library pendukung terpasang:
   * `RTClib` (Adafruit)
   * `DHT sensor library` (Adafruit)
   * `ModbusMaster` (Doc Walker)
   * `INA226_WE` (Wolfgang Ewald)
5. Lakukan **Verify** dan **Upload** via kabel USB Type-C.

### B. Menjalankan Monitoring UDP Lokal di Laptop
1. Hubungkan laptop ke hotspot/Wi-Fi yang sama dengan ESP32 (`PKMMARGOREJO`).
2. Buka Terminal / Command Prompt dan jalankan:
   ```bash
   python cek_udp.py
   ```
3. Data seluruh sensor dari Node 1 dan Node 2 akan langsung tampil berurutan secara vertikal setiap detik.

---

## 👥 7. Tim Pengembang & Pengabdian

* **Dosen Pembimbing:** Nur Achmad Sulistyo Putro, S.Si., M.Cs., Ph.D.
* **Pelaksana:** Tim Program Pengabdian kepada Masyarakat (PKM) Kalurahan Margorejo 2026
* **Institusi:** Departemen Ilmu Komputer dan Elektronika (DIKE), Fakultas MIPA, Universitas Gadjah Mada

---

## 📄 Lisensi
Proyek ini didistribusikan di bawah lisensi **MIT License** untuk kepentingan pendidikan, riset, dan pengabdian masyarakat.
