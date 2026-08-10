#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>
#include <ModbusMaster.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// =====================================================
// KONFIGURASI THINGSPEAK
// =====================================================

const char* THINGSPEAK_WRITE_KEY =
    "IHSIHGGDMO77YQV0";

const char* THINGSPEAK_URL =
    "https://api.thingspeak.com/update";

// Minimum 15 detik. 60 detik lebih aman.
const unsigned long INTERVAL_THINGSPEAK =
    30000;

unsigned long waktuThingSpeakTerakhir =
    0;
// =====================================================
// IDENTITAS PERANGKAT
// =====================================================

const char* DEVICE_ID =
    "ESP32-S3-AGRI-01";

// =====================================================
// KONFIGURASI HOTSPOT HP
// =====================================================

const char* WIFI_SSID =
    "MONITORING-ESP32";

const char* WIFI_PASSWORD =
    "esp32monitor";

// Port penerima di laptop
const uint16_t UDP_DESTINATION_PORT =
    4210;

// Port lokal ESP32
const uint16_t UDP_LOCAL_PORT =
    4211;

// true = memakai broadcast
const bool UDP_USE_BROADCAST =
    true;

// Digunakan jika broadcast dimatikan
IPAddress UDP_LAPTOP_IP(
    192,
    168,
    1,
    100
);

// Interval pengiriman UDP
const unsigned long INTERVAL_UDP =
    1000;


/*
  PERBAIKAN:
  Interval reconnect diperbesar ke 10 detik agar
  tidak mengganggu proses reconnect yang sedang berjalan.
  Sebelumnya 1000 ms terlalu cepat.
*/
const unsigned long INTERVAL_WIFI_RECONNECT =
    10000;

// =====================================================
// KONFIGURASI GPIO ESP32-S3
// =====================================================

// LDR
#define LDR_PIN 4

// Relay
#define RELAY_AKTUATOR_PIN 5
#define RELAY_LAMPU_PIN 15
#define RELAY_KIPAS_PIN 39

// DHT22
#define DHT_LUAR_1_PIN 6
#define DHT_DALAM_PIN 7
#define DHT_LUAR_2_PIN 16

// RTC DS3231
#define SDA_RTC 8
#define SCL_RTC 9

// Sensor kelembapan tanah
#define SOIL_1_PIN 10
#define SOIL_2_PIN 11
#define SOIL_3_PIN 12

// MAX485
#define RS485_DIR_PIN 14
#define RS485_TX_PIN 17
#define RS485_RX_PIN 18

// =====================================================
// JENIS SENSOR DHT
// =====================================================

#define DHT_TYPE DHT22

// =====================================================
// PENGATURAN RTC
// =====================================================

#define SET_RTC_ONCE false

// =====================================================
// LOGIKA FAN
// =====================================================

/*
  PERBAIKAN:
  Menggunakan satu batas suhu saja (28 C).
  Fan ON  jika suhu panel >= 28 C
  Fan OFF jika suhu panel <  28 C
*/
const float SUHU_KIPAS_ON =
    28.0;

// =====================================================
// JADWAL LAMPU
// =====================================================

const int JAM_LAMPU_ON =
    18;

const int JAM_LAMPU_OFF =
    6;

// =====================================================
// LOGIKA KELEMBAPAN TANAH
// =====================================================

// Kering jika <= 35 persen
const float BATAS_KERING =
    35.0;

// Basah jika >= 55 persen
const float BATAS_BASAH =
    55.0;

// Minimal 2 sensor kering
const uint8_t JUMLAH_KERING_BUKA =
    2;

// Minimal 2 sensor basah
const uint8_t JUMLAH_BASAH_TUTUP =
    2;

// Valve maksimal terbuka 10 menit
const unsigned long MAKSIMUM_WAKTU_VALVE =
    10UL * 60UL * 1000UL;

// Jeda valve 1 menit
const unsigned long JEDA_MINIMUM_VALVE =
    60UL * 1000UL;

// =====================================================
// KALIBRASI SENSOR KELEMBAPAN TANAH
// =====================================================

/*
  KALIBRASI PER-SENSOR

  Setiap sensor kapasitif memiliki karakteristik
  ADC yang berbeda-beda. Oleh karena itu kalibrasi
  dilakukan secara individual.

  Hasil pengukuran nyata:

  Sensor 1 (GPIO10):
    Di udara       = 3639
    Di tanah kering = 3495 sampai 3524

  Sensor 2 (GPIO11):
    Di udara       = 4095
    Di tanah kering = 3911 sampai 3964

  Sensor 3 (GPIO12):
    Di udara       = 4095
    Di tanah kering = 3959 sampai 3977

  nilaiKering diatur sedikit di atas pembacaan
  tanah kering tertinggi. Nilai di atas nilaiKering
  akan dipetakan ke 0 persen dan difilter.

  nilaiBasah belum dikalibrasi basah, untuk
  sementara menggunakan estimasi 1250.
  Perbarui setelah pengujian di tanah basah.
*/

const int nilaiKering[3] = {
    3550,
    3970,
    4050
};

const int nilaiBasah[3] = {
    1250,
    1250,
    1250
};

/*
  Nilai yang terlalu dekat 0 atau 4095
  dianggap pembacaan tidak valid.
*/

const int ADC_MIN_VALID =
    20;

const int ADC_MAX_VALID =
    4075;

/*
  Batas minimum persentase kelembapan.

  Jika hasil konversi <= 1 persen, sensor dianggap
  tidak valid (kemungkinan tidak terhubung dengan
  baik atau berada di udara terbuka).

  Sensor seperti ini tidak ikut dalam voting
  untuk membuka atau menutup valve.
*/

const float BATAS_PERSEN_MINIMUM =
    1.0;

/*
  PERBAIKAN:
  ESP32-S3 TIDAK memiliki konflik ADC2 vs WiFi
  seperti ESP32 klasik. Oleh karena itu WiFi
  TIDAK PERLU dihentikan saat membaca sensor tanah.

  Variabel HENTIKAN_WIFI_SAAT_BACA_SOIL dan fungsi
  hentikanWifiUntukSoil / aktifkanWifiSetelahSoil
  telah dihapus karena justru menyebabkan WiFi
  putus-sambung terus-menerus.
*/

/*
  Pembacaan tanah setiap 10 detik.
*/

const unsigned long INTERVAL_SOIL =
    10000;

// =====================================================
// KONFIGURASI MODBUS NPK
// =====================================================

const uint32_t NPK_BAUD_RATE =
    4800;

const uint8_t NPK_SLAVE_ID =
    1;

const uint16_t NPK_START_REGISTER =
    0x0000;

const uint8_t NPK_REGISTER_COUNT =
    7;

// false = function 03
// true  = function 04
const bool NPK_INPUT_REGISTER =
    false;

// =====================================================
// INTERVAL SENSOR
// =====================================================

const unsigned long INTERVAL_DHT_DALAM =
    1000;

const unsigned long INTERVAL_RTC =
    1000;

const unsigned long INTERVAL_DHT_LUAR =
    1000;

const unsigned long INTERVAL_LDR =
    1000;

const unsigned long INTERVAL_NPK =
    1000;

const unsigned long INTERVAL_TAMPIL =
    1000;

// =====================================================
// OBJEK SENSOR
// =====================================================

RTC_DS3231 rtc;

DHT dhtLuar1(
    DHT_LUAR_1_PIN,
    DHT_TYPE
);

DHT dhtDalam(
    DHT_DALAM_PIN,
    DHT_TYPE
);

DHT dhtLuar2(
    DHT_LUAR_2_PIN,
    DHT_TYPE
);

HardwareSerial rs485Serial(1);

ModbusMaster npkNode;

WiFiUDP udp;

// =====================================================
// STATUS SISTEM
// =====================================================

bool rtcTerdeteksi =
    false;

bool udpSiap =
    false;

IPAddress alamatBroadcast;

// Status relay
bool kipasNyala =
    false;

bool lampuNyala =
    false;

bool valveTerbuka =
    false;

bool valveTimeout =
    false;

// Waktu valve
unsigned long waktuValveMulai =
    0;

unsigned long waktuValveBerhenti =
    0;

// =====================================================
// DATA DHT
// =====================================================

float suhuDalam =
    NAN;

float lembapDalam =
    NAN;

float suhuLuar1 =
    NAN;

float lembapLuar1 =
    NAN;

float suhuLuar2 =
    NAN;

float lembapLuar2 =
    NAN;

float suhuLuarRata =
    NAN;

float lembapLuarRata =
    NAN;

// =====================================================
// DATA LDR
// =====================================================

int nilaiLDR =
    0;

// =====================================================
// DATA SENSOR KELEMBAPAN TANAH
// =====================================================

int nilaiSoil[3] = {
  0,
  0,
  0
};

float persenSoil[3] = {
  -1.0,
  -1.0,
  -1.0
};

bool soilValid[3] = {
  false,
  false,
  false
};

float rataRataKelembapanTanah =
    -1.0;

// =====================================================
// DATA NPK
// =====================================================

struct DataNPK {
  float kelembapan =
      NAN;

  float suhu =
      NAN;

  uint16_t ec =
      0;

  float ph =
      NAN;

  uint16_t nitrogen =
      0;

  uint16_t phosphorus =
      0;

  uint16_t potassium =
      0;

  bool valid =
      false;

  uint8_t kodeModbus =
      0xFF;
};

DataNPK dataNpk;

// =====================================================
// TIMER
// =====================================================

unsigned long waktuDhtDalamTerakhir =
    0;

unsigned long waktuRtcTerakhir =
    0;

unsigned long waktuDhtLuarTerakhir =
    0;

unsigned long waktuLdrTerakhir =
    0;

unsigned long waktuSoilTerakhir =
    0;

unsigned long waktuNpkTerakhir =
    0;

unsigned long waktuTampilTerakhir =
    0;

unsigned long waktuUdpTerakhir =
    0;

unsigned long waktuWifiTerakhir =
    0;

uint32_t nomorPaketUdp =
    0;

// =====================================================
// FUNGSI DUA DIGIT
// =====================================================

void print2Digits(int angka) {
  if (angka < 10) {
    Serial.print("0");
  }

  Serial.print(angka);
}

// =====================================================
// KONDISI CAHAYA
// =====================================================

const char* kondisiCahaya(
    int nilai
) {
  if (nilai < 800) {
    return "Sangat Terang";
  }
  else if (nilai < 1600) {
    return "Terang";
  }
  else if (nilai < 2600) {
    return "Cukup Terang";
  }
  else if (nilai < 3400) {
    return "Gelap";
  }

  return "Sangat Gelap";
}

// =====================================================
// KONTROL RELAY ACTIVE HIGH
// =====================================================

void setRelay(
    uint8_t pinRelay,
    bool status
) {
  digitalWrite(
      pinRelay,
      status ? HIGH : LOW
  );
}

void setKipas(bool status) {
  kipasNyala =
      status;

  setRelay(
      RELAY_KIPAS_PIN,
      status
  );
}

void setLampu(bool status) {
  lampuNyala =
      status;

  setRelay(
      RELAY_LAMPU_PIN,
      status
  );
}

void setValve(bool status) {
  valveTerbuka =
      status;

  setRelay(
      RELAY_AKTUATOR_PIN,
      status
  );
}

// =====================================================
// JADWAL LAMPU
// =====================================================

bool jadwalLampuNyala(
    int jamSekarang
) {
  return (
      jamSekarang >=
      JAM_LAMPU_ON ||
      jamSekarang <
      JAM_LAMPU_OFF
  );
}

// =====================================================
// PEMBACAAN ADC LDR
// =====================================================

int bacaAdcRataRata(
    uint8_t pin,
    uint8_t jumlahSampel = 16
) {
  uint32_t total =
      0;

  for (
      uint8_t i = 0;
      i < jumlahSampel;
      i++
  ) {
    total +=
        analogRead(pin);

    delayMicroseconds(250);
  }

  return static_cast<int>(
      total /
      jumlahSampel
  );
}

// =====================================================
// PEMBACAAN ADC SOIL STABIL
// =====================================================

int bacaAdcSoilStabil(
    uint8_t pin
) {
  const uint8_t JUMLAH_SAMPEL =
      15;

  uint16_t sampel[
      JUMLAH_SAMPEL
  ];

  /*
    Buang pembacaan pertama setelah
    berpindah kanal ADC.
  */

  for (
      uint8_t i = 0;
      i < 3;
      i++
  ) {
    analogRead(pin);

    delayMicroseconds(500);
  }

  /*
    Ambil 15 sampel.
  */

  for (
      uint8_t i = 0;
      i < JUMLAH_SAMPEL;
      i++
  ) {
    sampel[i] =
        analogRead(pin);

    delayMicroseconds(700);
  }

  /*
    Urutkan data.
  */

  for (
      uint8_t i = 1;
      i < JUMLAH_SAMPEL;
      i++
  ) {
    uint16_t nilaiSementara =
        sampel[i];

    int8_t j =
        i - 1;

    while (
        j >= 0 &&
        sampel[j] >
        nilaiSementara
    ) {
      sampel[j + 1] =
          sampel[j];

      j--;
    }

    sampel[j + 1] =
        nilaiSementara;
  }

  /*
    Gunakan nilai median.
  */

  return sampel[
      JUMLAH_SAMPEL / 2
  ];
}

// =====================================================
// KONVERSI ADC KE PERSENTASE
// =====================================================

float konversiKePersen(
    int nilaiAnalog,
    uint8_t indeksSensor
) {
  int nilaiDibatasi =
      constrain(
          nilaiAnalog,
          nilaiBasah[indeksSensor],
          nilaiKering[indeksSensor]
      );

  /*
    Rumus mapping per sensor:

    nilaiKering[i] = 0 persen
    nilaiBasah[i]  = 100 persen
  */

  float persen =
      (
        static_cast<float>(
            nilaiKering[indeksSensor] -
            nilaiDibatasi
        ) *
        100.0
      ) /
      static_cast<float>(
          nilaiKering[indeksSensor] -
          nilaiBasah[indeksSensor]
      );

  return constrain(
      persen,
      0.0,
      100.0
  );
}

bool nilaiSoilValid(
    int nilai
) {
  return (
      nilai >
      ADC_MIN_VALID &&
      nilai <
      ADC_MAX_VALID
  );
}

// =====================================================
// BACA SENSOR KELEMBAPAN TANAH
// =====================================================

/*
  PERBAIKAN:
  WiFi tidak perlu dihentikan pada ESP32-S3.
  Fungsi hentikanWifiUntukSoil dan
  aktifkanWifiSetelahSoil telah dihapus.
*/

void bacaSensorKelembapan() {
  /*
    GPIO tetap:

    Sensor 1 = GPIO10
    Sensor 2 = GPIO11
    Sensor 3 = GPIO12
  */

  nilaiSoil[0] =
      bacaAdcSoilStabil(
          SOIL_1_PIN
      );

  delayMicroseconds(1000);

  nilaiSoil[1] =
      bacaAdcSoilStabil(
          SOIL_2_PIN
      );

  delayMicroseconds(1000);

  nilaiSoil[2] =
      bacaAdcSoilStabil(
          SOIL_3_PIN
      );

  soilValid[0] =
      nilaiSoilValid(
          nilaiSoil[0]
      );

  soilValid[1] =
      nilaiSoilValid(
          nilaiSoil[1]
      );

  soilValid[2] =
      nilaiSoilValid(
          nilaiSoil[2]
      );

  float totalPersen =
      0.0;

  uint8_t jumlahValid =
      0;

  for (
      uint8_t i = 0;
      i < 3;
      i++
  ) {
    if (soilValid[i]) {
      persenSoil[i] =
          konversiKePersen(
              nilaiSoil[i],
              i
          );

      /*
        Jika persentase <= 1 persen,
        sensor dianggap tidak valid
        (kemungkinan tidak terhubung
        atau berada di udara terbuka).
      */

      if (
          persenSoil[i] <=
          BATAS_PERSEN_MINIMUM
      ) {
        soilValid[i] =
            false;

        persenSoil[i] =
            -1.0;
      }
      else {
        totalPersen +=
            persenSoil[i];

        jumlahValid++;
      }
    }
    else {
      persenSoil[i] =
          -1.0;
    }
  }

  if (jumlahValid > 0) {
    rataRataKelembapanTanah =
        totalPersen /
        static_cast<float>(
            jumlahValid
        );
  }
  else {
    rataRataKelembapanTanah =
        -1.0;
  }
}

// =====================================================
// KONTROL VALVE 2 DARI 3 SENSOR
// =====================================================

void kontrolValve(
    unsigned long sekarang
) {
  uint8_t jumlahValid =
      0;

  uint8_t jumlahKering =
      0;

  uint8_t jumlahBasah =
      0;

  for (
      uint8_t i = 0;
      i < 3;
      i++
  ) {
    if (!soilValid[i]) {
      continue;
    }

    jumlahValid++;

    if (
        persenSoil[i] <=
        BATAS_KERING
    ) {
      jumlahKering++;
    }

    if (
        persenSoil[i] >=
        BATAS_BASAH
    ) {
      jumlahBasah++;
    }
  }

  /*
    Minimal dua sensor harus valid.
  */

  if (jumlahValid < 2) {
    if (valveTerbuka) {
      setValve(false);

      waktuValveBerhenti =
          sekarang;
    }

    return;
  }

  if (valveTerbuka) {
    bool minimalDuaSensorBasah =
        jumlahBasah >=
        JUMLAH_BASAH_TUTUP;

    bool waktuMaksimumTercapai =
        sekarang -
        waktuValveMulai >=
        MAKSIMUM_WAKTU_VALVE;

    if (
        minimalDuaSensorBasah ||
        waktuMaksimumTercapai
    ) {
      setValve(false);

      waktuValveBerhenti =
          sekarang;

      if (
          waktuMaksimumTercapai
      ) {
        valveTimeout =
            true;
      }
    }
  }
  else {
    if (
        jumlahBasah >=
        JUMLAH_BASAH_TUTUP
    ) {
      valveTimeout =
          false;
    }

    bool minimalDuaSensorKering =
        jumlahKering >=
        JUMLAH_KERING_BUKA;

    bool jedaSelesai =
        sekarang -
        waktuValveBerhenti >=
        JEDA_MINIMUM_VALVE;

    if (
        minimalDuaSensorKering &&
        jedaSelesai &&
        !valveTimeout
    ) {
      setValve(true);

      waktuValveMulai =
          sekarang;
    }
  }
}

// =====================================================
// KONTROL ARAH MAX485
// =====================================================

void rs485PreTransmission() {
  digitalWrite(
      RS485_DIR_PIN,
      HIGH
  );

  delayMicroseconds(150);
}

void rs485PostTransmission() {
  rs485Serial.flush();

  delayMicroseconds(150);

  digitalWrite(
      RS485_DIR_PIN,
      LOW
  );
}

// =====================================================
// BACA SENSOR NPK
// =====================================================

void bacaSensorNpk() {
  uint8_t hasil;

  if (NPK_INPUT_REGISTER) {
    hasil =
        npkNode.readInputRegisters(
            NPK_START_REGISTER,
            NPK_REGISTER_COUNT
        );
  }
  else {
    hasil =
        npkNode.readHoldingRegisters(
            NPK_START_REGISTER,
            NPK_REGISTER_COUNT
        );
  }

  dataNpk.kodeModbus =
      hasil;

  if (
      hasil !=
      ModbusMaster::ku8MBSuccess
  ) {
    dataNpk.valid =
        false;

    return;
  }

  uint16_t rawKelembapan =
      npkNode.getResponseBuffer(0);

  int16_t rawSuhu =
      static_cast<int16_t>(
          npkNode.getResponseBuffer(1)
      );

  uint16_t rawEc =
      npkNode.getResponseBuffer(2);

  uint16_t rawPh =
      npkNode.getResponseBuffer(3);

  uint16_t rawNitrogen =
      npkNode.getResponseBuffer(4);

  uint16_t rawPhosphorus =
      npkNode.getResponseBuffer(5);

  uint16_t rawPotassium =
      npkNode.getResponseBuffer(6);

  dataNpk.kelembapan =
      rawKelembapan /
      10.0;

  dataNpk.suhu =
      rawSuhu /
      10.0;

  dataNpk.ec =
      rawEc;

  dataNpk.ph =
      rawPh /
      10.0;

  dataNpk.nitrogen =
      rawNitrogen;

  dataNpk.phosphorus =
      rawPhosphorus;

  dataNpk.potassium =
      rawPotassium;

  dataNpk.valid =
      true;
}

// =====================================================
// ALAMAT BROADCAST
// =====================================================

IPAddress hitungAlamatBroadcast() {
  IPAddress ip =
      WiFi.localIP();

  IPAddress subnet =
      WiFi.subnetMask();

  IPAddress broadcast;

  for (
      uint8_t i = 0;
      i < 4;
      i++
  ) {
    broadcast[i] =
        ip[i] |
        static_cast<uint8_t>(
            ~subnet[i]
        );
  }

  return broadcast;
}

// =====================================================
// MEMULAI UDP
// =====================================================

void mulaiUdp() {
  udp.stop();

  udpSiap =
      udp.begin(
          UDP_LOCAL_PORT
      );

  if (udpSiap) {
    alamatBroadcast =
        hitungAlamatBroadcast();

    Serial.println(
        "UDP berhasil dimulai."
    );

    Serial.print(
        "Alamat broadcast: "
    );

    Serial.println(
        alamatBroadcast
    );

    Serial.print(
        "Port tujuan: "
    );

    Serial.println(
        UDP_DESTINATION_PORT
    );
  }
  else {
    Serial.println(
        "UDP gagal dimulai."
    );
  }
}

// =====================================================
// MULAI WIFI
// =====================================================

void mulaiWifi() {
  Serial.println();

  Serial.print(
      "Menghubungkan ke hotspot HP: "
  );

  Serial.println(
      WIFI_SSID
  );

  WiFi.mode(
      WIFI_STA
  );

  WiFi.setSleep(
      false
  );

  /*
    PERBAIKAN:
    Aktifkan auto-reconnect bawaan ESP32 agar
    driver WiFi menangani reconnect secara otomatis.
  */
  WiFi.setAutoReconnect(
      true
  );

  WiFi.begin(
      WIFI_SSID,
      WIFI_PASSWORD
  );

  unsigned long waktuMulai =
      millis();

  while (
      WiFi.status() !=
      WL_CONNECTED &&
      millis() -
      waktuMulai <
      20000
  ) {
    delay(500);

    Serial.print(".");
  }

  Serial.println();

  if (
      WiFi.status() ==
      WL_CONNECTED
  ) {
    Serial.println(
        "Hotspot HP berhasil terhubung."
    );

    Serial.print(
        "IP ESP32: "
    );

    Serial.println(
        WiFi.localIP()
    );

    Serial.print(
        "Subnet mask: "
    );

    Serial.println(
        WiFi.subnetMask()
    );

    Serial.print(
        "Gateway HP: "
    );

    Serial.println(
        WiFi.gatewayIP()
    );

    Serial.print(
        "Kekuatan Wi-Fi: "
    );

    Serial.print(
        WiFi.RSSI()
    );

    Serial.println(
        " dBm"
    );

    mulaiUdp();
  }
  else {
    udpSiap =
        false;

    Serial.println(
        "ESP32 belum berhasil terhubung."
    );

    Serial.println(
        "Sensor dan relay tetap bekerja secara lokal."
    );
  }
}

// =====================================================
// RECONNECT WIFI
// =====================================================

/*
  PERBAIKAN:
  1. WiFi.disconnect() dihapus karena membatalkan
     proses reconnect yang sedang berjalan.
  2. Interval diperbesar ke 10 detik.
  3. WiFi.setAutoReconnect(true) sudah menangani
     reconnect di level driver, fungsi ini hanya
     sebagai backup.
*/

void periksaWifi(
    unsigned long sekarang
) {
  if (
      WiFi.status() ==
      WL_CONNECTED
  ) {
    if (!udpSiap) {
      mulaiUdp();
    }

    return;
  }

  udpSiap =
      false;

  if (
      sekarang -
      waktuWifiTerakhir <
      INTERVAL_WIFI_RECONNECT
  ) {
    return;
  }

  waktuWifiTerakhir =
      sekarang;

  Serial.println(
      "Wi-Fi terputus, mencoba kembali..."
  );

  WiFi.begin(
      WIFI_SSID,
      WIFI_PASSWORD
  );
}

// =====================================================
// FORMAT JSON
// =====================================================

String floatKeJson(
    float nilai,
    unsigned int desimal = 1
) {
  if (isnan(nilai)) {
    return "null";
  }

  return String(
      nilai,
      desimal
  );
}

const char* boolKeJson(
    bool nilai
) {
  return nilai ?
      "true" :
      "false";
}

// =====================================================
// MEMBUAT PAYLOAD UDP
// =====================================================

String buatPayloadUdp() {
  nomorPaketUdp++;

  String payload;

  payload.reserve(1300);

  char waktuRtc[24];

  if (rtcTerdeteksi) {
    DateTime now =
        rtc.now();

    snprintf(
        waktuRtc,
        sizeof(waktuRtc),
        "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(),
        now.month(),
        now.day(),
        now.hour(),
        now.minute(),
        now.second()
    );
  }
  else {
    snprintf(
        waktuRtc,
        sizeof(waktuRtc),
        "RTC_ERROR"
    );
  }

  payload += "{";

  payload += "\"device_id\":\"";
  payload += DEVICE_ID;
  payload += "\",";

  payload += "\"seq\":";
  payload += String(nomorPaketUdp);
  payload += ",";

  payload += "\"rtc\":\"";
  payload += waktuRtc;
  payload += "\",";

  payload += "\"esp_ip\":\"";
  payload += WiFi.localIP().toString();
  payload += "\",";

  payload += "\"wifi_rssi\":";
  payload += String(WiFi.RSSI());
  payload += ",";

  payload += "\"suhu_luar1\":";
  payload += floatKeJson(suhuLuar1);
  payload += ",";

  payload += "\"lembap_luar1\":";
  payload += floatKeJson(lembapLuar1);
  payload += ",";

  payload += "\"suhu_luar2\":";
  payload += floatKeJson(suhuLuar2);
  payload += ",";

  payload += "\"lembap_luar2\":";
  payload += floatKeJson(lembapLuar2);
  payload += ",";

  payload += "\"suhu_luar_rata\":";
  payload += floatKeJson(suhuLuarRata);
  payload += ",";

  payload += "\"lembap_luar_rata\":";
  payload += floatKeJson(lembapLuarRata);
  payload += ",";

  payload += "\"suhu_dalam\":";
  payload += floatKeJson(suhuDalam);
  payload += ",";

  payload += "\"lembap_dalam\":";
  payload += floatKeJson(lembapDalam);
  payload += ",";

  payload += "\"ldr_raw\":";
  payload += String(nilaiLDR);
  payload += ",";

  payload += "\"cahaya\":\"";
  payload += kondisiCahaya(nilaiLDR);
  payload += "\",";

  payload += "\"soil1_raw\":";
  payload += String(nilaiSoil[0]);
  payload += ",";

  payload += "\"soil1_pct\":";

  payload += soilValid[0] ?
      String(persenSoil[0], 1) :
      "null";

  payload += ",";

  payload += "\"soil1_valid\":";
  payload += boolKeJson(soilValid[0]);
  payload += ",";

  payload += "\"soil2_raw\":";
  payload += String(nilaiSoil[1]);
  payload += ",";

  payload += "\"soil2_pct\":";

  payload += soilValid[1] ?
      String(persenSoil[1], 1) :
      "null";

  payload += ",";

  payload += "\"soil2_valid\":";
  payload += boolKeJson(soilValid[1]);
  payload += ",";

  payload += "\"soil3_raw\":";
  payload += String(nilaiSoil[2]);
  payload += ",";

  payload += "\"soil3_pct\":";

  payload += soilValid[2] ?
      String(persenSoil[2], 1) :
      "null";

  payload += ",";

  payload += "\"soil3_valid\":";
  payload += boolKeJson(soilValid[2]);
  payload += ",";

  payload += "\"npk_valid\":";
  payload += boolKeJson(dataNpk.valid);
  payload += ",";

  payload += "\"npk_error\":";
  payload += String(dataNpk.kodeModbus);
  payload += ",";

  payload += "\"npk_kelembapan\":";

  payload += dataNpk.valid ?
      String(dataNpk.kelembapan, 1) :
      "null";

  payload += ",";

  payload += "\"npk_suhu\":";

  payload += dataNpk.valid ?
      String(dataNpk.suhu, 1) :
      "null";

  payload += ",";

  payload += "\"npk_ec\":";

  payload += dataNpk.valid ?
      String(dataNpk.ec) :
      "null";

  payload += ",";

  payload += "\"npk_ph\":";

  payload += dataNpk.valid ?
      String(dataNpk.ph, 1) :
      "null";

  payload += ",";

  payload += "\"npk_n\":";

  payload += dataNpk.valid ?
      String(dataNpk.nitrogen) :
      "null";

  payload += ",";

  payload += "\"npk_p\":";

  payload += dataNpk.valid ?
      String(dataNpk.phosphorus) :
      "null";

  payload += ",";

  payload += "\"npk_k\":";

  payload += dataNpk.valid ?
      String(dataNpk.potassium) :
      "null";

  payload += ",";

  payload += "\"kipas\":";
  payload += boolKeJson(kipasNyala);
  payload += ",";

  payload += "\"lampu\":";
  payload += boolKeJson(lampuNyala);
  payload += ",";

  payload += "\"valve\":";
  payload += boolKeJson(valveTerbuka);
  payload += ",";

  payload += "\"valve_timeout\":";
  payload += boolKeJson(valveTimeout);

  payload += "}";

  return payload;
}

// =====================================================
// KIRIM DATA UDP
// =====================================================

void kirimDataUdp() {
  if (
      WiFi.status() !=
      WL_CONNECTED
  ) {
    return;
  }

  if (!udpSiap) {
    return;
  }

  String payload =
      buatPayloadUdp();

  IPAddress alamatTujuan;

  if (UDP_USE_BROADCAST) {
    alamatTujuan =
        alamatBroadcast;
  }
  else {
    alamatTujuan =
        UDP_LAPTOP_IP;
  }

  int mulaiPaket =
      udp.beginPacket(
          alamatTujuan,
          UDP_DESTINATION_PORT
      );

  if (!mulaiPaket) {
    Serial.println(
        "UDP gagal memulai paket."
    );

    return;
  }

  udp.write(
      reinterpret_cast<
          const uint8_t*
      >(
          payload.c_str()
      ),
      payload.length()
  );

  int hasil =
      udp.endPacket();

  if (hasil == 1) {
    Serial.print(
        "UDP terkirim | seq="
    );

    Serial.print(
        nomorPaketUdp
    );

    Serial.print(
        " | tujuan="
    );

    Serial.print(
        alamatTujuan
    );

    Serial.print(":");

    Serial.print(
        UDP_DESTINATION_PORT
    );

    Serial.print(
        " | ukuran="
    );

    Serial.print(
        payload.length()
    );

    Serial.println(
        " byte"
    );
  }
  else {
    Serial.println(
        "UDP gagal dikirim."
    );
  }
}

void kirimDataThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(
        "ThingSpeak: WiFi tidak terhubung, dilewati."
    );

    return;
  }

  String body =
      "api_key=";

  body += THINGSPEAK_WRITE_KEY;

  /*
    field1 sampai field5: data NPK.
    Seluruhnya dilewati jika pembacaan Modbus gagal.
  */

  if (dataNpk.valid) {
    body += "&field1=";
    body += String(dataNpk.nitrogen);

    body += "&field2=";
    body += String(dataNpk.phosphorus);

    body += "&field3=";
    body += String(dataNpk.potassium);

    body += "&field4=";
    body += String(dataNpk.ph, 2);

    body += "&field5=";
    body += String(dataNpk.ec);
  }

  /*
    field6: rata-rata kelembapan tanah.
    Bernilai -1.0 jika tidak ada sensor yang valid.
  */

  if (rataRataKelembapanTanah >= 0.0) {
    body += "&field6=";
    body += String(rataRataKelembapanTanah, 1);
  }

  /*
    field7 dan field8: suhu dan kelembapan luar.
  */

  if (!isnan(suhuLuarRata)) {
    body += "&field7=";
    body += String(suhuLuarRata, 2);
  }

  if (!isnan(lembapLuarRata)) {
    body += "&field8=";
    body += String(lembapLuarRata, 2);
  }

  /*
    Jika tidak ada satu pun sensor yang valid,
    tidak perlu mengirim apa pun.
  */

  if (body.indexOf("&field") < 0) {
    Serial.println(
        "ThingSpeak: tidak ada data valid, dilewati."
    );

    return;
  }

  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient http;

  http.setTimeout(5000);

  if (!http.begin(client, THINGSPEAK_URL)) {
    Serial.println(
        "ThingSpeak: gagal memulai koneksi."
    );

    return;
  }

  http.addHeader(
      "Content-Type",
      "application/x-www-form-urlencoded"
  );

  int kode =
      http.POST(body);

  if (kode > 0) {
    String balasan =
        http.getString();

    balasan.trim();

    Serial.print("ThingSpeak: HTTP ");
    Serial.print(kode);
    Serial.print(" -> entry ");
    Serial.println(balasan);

    if (balasan == "0") {
      Serial.println(
          "ThingSpeak: DITOLAK. Periksa write key "
          "atau interval pengiriman."
      );
    }
  }
  else {
    Serial.print("ThingSpeak: error ");
    Serial.println(kode);
  }

  http.end();
}
// =====================================================
// TAMPIL DATA SERIAL
// =====================================================

void tampilkanDataSerial() {
  Serial.println();

  Serial.println(
      "---------------- DATA SISTEM ----------------"
  );

  if (rtcTerdeteksi) {
    DateTime now =
        rtc.now();

    Serial.print(
        "Tanggal             : "
    );

    print2Digits(
        now.day()
    );

    Serial.print("/");

    print2Digits(
        now.month()
    );

    Serial.print("/");

    Serial.println(
        now.year()
    );

    Serial.print(
        "Waktu               : "
    );

    print2Digits(
        now.hour()
    );

    Serial.print(":");

    print2Digits(
        now.minute()
    );

    Serial.print(":");

    print2Digits(
        now.second()
    );

    Serial.println();
  }
  else {
    Serial.println(
        "RTC                 : ERROR"
    );
  }

  Serial.print(
      "Wi-Fi               : "
  );

  if (
      WiFi.status() ==
      WL_CONNECTED
  ) {
    Serial.print(
        "CONNECTED | IP "
    );

    Serial.print(
        WiFi.localIP()
    );

    Serial.print(
        " | RSSI "
    );

    Serial.print(
        WiFi.RSSI()
    );

    Serial.println(
        " dBm"
    );
  }
  else {
    Serial.println(
        "DISCONNECTED"
    );
  }

  Serial.print(
      "LDR                 : "
  );

  Serial.print(
      nilaiLDR
  );

  Serial.print(
      " | "
  );

  Serial.println(
      kondisiCahaya(
          nilaiLDR
      )
  );

  Serial.println(
      "---------------------------------------------"
  );

  if (
      !isnan(suhuLuar1) &&
      !isnan(lembapLuar1)
  ) {
    Serial.print(
        "Suhu Luar 1         : "
    );

    Serial.print(
        suhuLuar1,
        1
    );

    Serial.println(
        " C"
    );

    Serial.print(
        "Lembap Luar 1       : "
    );

    Serial.print(
        lembapLuar1,
        1
    );

    Serial.println(
        " %"
    );
  }
  else {
    Serial.println(
        "DHT Luar 1          : Gagal terbaca"
    );
  }

  if (
      !isnan(suhuLuar2) &&
      !isnan(lembapLuar2)
  ) {
    Serial.print(
        "Suhu Luar 2         : "
    );

    Serial.print(
        suhuLuar2,
        1
    );

    Serial.println(
        " C"
    );

    Serial.print(
        "Lembap Luar 2       : "
    );

    Serial.print(
        lembapLuar2,
        1
    );

    Serial.println(
        " %"
    );
  }
  else {
    Serial.println(
        "DHT Luar 2          : Gagal terbaca"
    );
  }

  if (
      !isnan(suhuDalam) &&
      !isnan(lembapDalam)
  ) {
    Serial.print(
        "Suhu Dalam Panel    : "
    );

    Serial.print(
        suhuDalam,
        1
    );

    Serial.println(
        " C"
    );

    Serial.print(
        "Lembap Dalam Panel  : "
    );

    Serial.print(
        lembapDalam,
        1
    );

    Serial.println(
        " %"
    );
  }
  else {
    Serial.println(
        "DHT Dalam           : Gagal terbaca"
    );
  }

  Serial.println(
      "---------------------------------------------"
  );

  for (
      uint8_t i = 0;
      i < 3;
      i++
  ) {
    Serial.print(
        "Sensor tanah "
    );

    Serial.print(
        i + 1
    );

    Serial.print(
        "     : ADC "
    );

    Serial.print(
        nilaiSoil[i]
    );

    Serial.print(
        " | "
    );

    if (soilValid[i]) {
      Serial.print(
          persenSoil[i],
          1
      );

      Serial.print(
          "% | VALID"
      );
    }
    else {
      Serial.print(
          "TIDAK TERBACA | ERROR"
      );
    }

    Serial.println();
  }

  Serial.print(
      "Rata-rata tanah     : "
  );

  if (
      rataRataKelembapanTanah >=
      0.0
  ) {
    Serial.print(
        rataRataKelembapanTanah,
        1
    );

    Serial.println(
        " %"
    );
  }
  else {
    Serial.println(
        "TIDAK TERSEDIA"
    );
  }

  Serial.println(
      "---------------------------------------------"
  );

  if (dataNpk.valid) {
    Serial.print(
        "NPK Kelembapan      : "
    );

    Serial.print(
        dataNpk.kelembapan,
        1
    );

    Serial.println(
        " %"
    );

    Serial.print(
        "NPK Suhu            : "
    );

    Serial.print(
        dataNpk.suhu,
        1
    );

    Serial.println(
        " C"
    );

    Serial.print(
        "NPK EC              : "
    );

    Serial.println(
        dataNpk.ec
    );

    Serial.print(
        "NPK pH              : "
    );

    Serial.println(
        dataNpk.ph,
        1
    );

    Serial.print(
        "N / P / K           : "
    );

    Serial.print(
        dataNpk.nitrogen
    );

    Serial.print(
        " / "
    );

    Serial.print(
        dataNpk.phosphorus
    );

    Serial.print(
        " / "
    );

    Serial.println(
        dataNpk.potassium
    );
  }
  else {
    Serial.print(
        "NPK                 : ERROR MODBUS "
    );

    Serial.println(
        dataNpk.kodeModbus
    );
  }

  Serial.println(
      "---------------------------------------------"
  );

  Serial.print(
      "Kipas               : "
  );

  Serial.println(
      kipasNyala ?
      "ON" :
      "OFF"
  );

  Serial.print(
      "Lampu               : "
  );

  Serial.println(
      lampuNyala ?
      "ON" :
      "OFF"
  );

  Serial.print(
      "Valve               : "
  );

  Serial.println(
      valveTerbuka ?
      "OPEN" :
      "CLOSED"
  );

  Serial.print(
      "Valve timeout       : "
  );

  Serial.println(
      valveTimeout ?
      "FAULT" :
      "NORMAL"
  );
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(
      115200
  );

  delay(1000);

  Serial.println();

  Serial.println(
      "=============================================="
  );

  Serial.println(
      " SISTEM MONITORING ESP32-S3 + UDP"
  );

  Serial.println(
      "=============================================="
  );

  // Relay
  pinMode(
      RELAY_KIPAS_PIN,
      OUTPUT
  );

  pinMode(
      RELAY_LAMPU_PIN,
      OUTPUT
  );

  pinMode(
      RELAY_AKTUATOR_PIN,
      OUTPUT
  );

  setKipas(false);
  setLampu(false);
  setValve(false);

  // ADC
  pinMode(
      LDR_PIN,
      INPUT
  );

  pinMode(
      SOIL_1_PIN,
      INPUT
  );

  pinMode(
      SOIL_2_PIN,
      INPUT
  );

  pinMode(
      SOIL_3_PIN,
      INPUT
  );

  analogReadResolution(
      12
  );

  analogSetPinAttenuation(
      LDR_PIN,
      ADC_11db
  );

  analogSetPinAttenuation(
      SOIL_1_PIN,
      ADC_11db
  );

  analogSetPinAttenuation(
      SOIL_2_PIN,
      ADC_11db
  );

  analogSetPinAttenuation(
      SOIL_3_PIN,
      ADC_11db
  );

  // DHT
  dhtLuar1.begin();
  dhtDalam.begin();
  dhtLuar2.begin();

  // RTC
  Wire.begin(
      SDA_RTC,
      SCL_RTC
  );

  if (!rtc.begin()) {
    rtcTerdeteksi =
        false;

    Serial.println(
        "RTC TIDAK TERDETEKSI."
    );

    setLampu(false);
  }
  else {
    rtcTerdeteksi =
        true;

    Serial.println(
        "RTC terdeteksi."
    );

    if (rtc.lostPower()) {
      Serial.println(
          "PERINGATAN: RTC pernah kehilangan daya."
      );
    }

    if (SET_RTC_ONCE) {
      rtc.adjust(
          DateTime(
              F(__DATE__),
              F(__TIME__)
          )
      );

      Serial.println(
          "RTC disetel dari waktu kompilasi."
      );

      Serial.println(
          "Ubah SET_RTC_ONCE menjadi false."
      );
    }
  }

  // MAX485
  pinMode(
      RS485_DIR_PIN,
      OUTPUT
  );

  digitalWrite(
      RS485_DIR_PIN,
      LOW
  );

  rs485Serial.begin(
      NPK_BAUD_RATE,
      SERIAL_8N1,
      RS485_RX_PIN,
      RS485_TX_PIN
  );

  npkNode.begin(
      NPK_SLAVE_ID,
      rs485Serial
  );

  npkNode.preTransmission(
      rs485PreTransmission
  );

  npkNode.postTransmission(
      rs485PostTransmission
  );

  /*
    Pembacaan pertama dilakukan sebelum
    WiFi dimulai.
  */

  bacaSensorKelembapan();

  // WiFi
  mulaiWifi();

  unsigned long sekarang =
      millis();

  waktuDhtDalamTerakhir =
      sekarang -
      INTERVAL_DHT_DALAM;

  waktuRtcTerakhir =
      sekarang -
      INTERVAL_RTC;

  waktuDhtLuarTerakhir =
      sekarang -
      INTERVAL_DHT_LUAR;

  waktuLdrTerakhir =
      sekarang -
      INTERVAL_LDR;

  /*
    Sensor tanah baru dibaca lagi
    setelah 10 detik.
  */

  waktuSoilTerakhir =
      sekarang;

  waktuNpkTerakhir =
      sekarang -
      INTERVAL_NPK;

  waktuTampilTerakhir =
      sekarang -
      INTERVAL_TAMPIL;

  waktuUdpTerakhir =
      sekarang -
      INTERVAL_UDP;

  waktuValveBerhenti =
      sekarang -
      JEDA_MINIMUM_VALVE;

  waktuThingSpeakTerakhir = 
      sekarang;
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  unsigned long sekarang =
      millis();

  // Periksa WiFi
  periksaWifi(
      sekarang
  );

  // ---------------------------------------------------
  // DHT DALAM DAN FAN
  // ---------------------------------------------------

  if (
      sekarang -
      waktuDhtDalamTerakhir >=
      INTERVAL_DHT_DALAM
  ) {
    waktuDhtDalamTerakhir =
        sekarang;

    suhuDalam =
        dhtDalam.readTemperature();

    lembapDalam =
        dhtDalam.readHumidity();

    bool dhtDalamValid =
        !isnan(suhuDalam) &&
        !isnan(lembapDalam);

    if (!dhtDalamValid) {
      setKipas(false);
    }
    else {
      /*
        PERBAIKAN LOGIKA FAN:
        Satu batas suhu saja (28 C).
        >= 28 C  = kipas ON
        <  28 C  = kipas OFF
      */
      if (
          suhuDalam >=
          SUHU_KIPAS_ON
      ) {
        setKipas(true);
      }
      else {
        setKipas(false);
      }
    }
  }

  // ---------------------------------------------------
  // RTC DAN LAMPU
  // ---------------------------------------------------

  if (
      sekarang -
      waktuRtcTerakhir >=
      INTERVAL_RTC
  ) {
    waktuRtcTerakhir =
        sekarang;

    if (rtcTerdeteksi) {
      DateTime now =
          rtc.now();

      setLampu(
          jadwalLampuNyala(
              now.hour()
          )
      );
    }
    else {
      setLampu(false);
    }
  }

  // ---------------------------------------------------
  // DHT LUAR
  // ---------------------------------------------------

  if (
      sekarang -
      waktuDhtLuarTerakhir >=
      INTERVAL_DHT_LUAR
  ) {
    waktuDhtLuarTerakhir =
        sekarang;

    suhuLuar1 =
        dhtLuar1.readTemperature();

    lembapLuar1 =
        dhtLuar1.readHumidity();

    suhuLuar2 =
        dhtLuar2.readTemperature();

    lembapLuar2 =
        dhtLuar2.readHumidity();

    bool dhtLuar1Valid =
        !isnan(suhuLuar1) &&
        !isnan(lembapLuar1);

    bool dhtLuar2Valid =
        !isnan(suhuLuar2) &&
        !isnan(lembapLuar2);

    if (
        dhtLuar1Valid &&
        dhtLuar2Valid
    ) {
      suhuLuarRata =
          (
            suhuLuar1 +
            suhuLuar2
          ) /
          2.0;

      lembapLuarRata =
          (
            lembapLuar1 +
            lembapLuar2
          ) /
          2.0;
    }
    else {
      suhuLuarRata =
          NAN;

      lembapLuarRata =
          NAN;
    }
  }

  // ---------------------------------------------------
  // LDR
  // ---------------------------------------------------

  if (
      sekarang -
      waktuLdrTerakhir >=
      INTERVAL_LDR
  ) {
    waktuLdrTerakhir =
        sekarang;

    nilaiLDR =
        bacaAdcRataRata(
            LDR_PIN
        );
  }

  // ---------------------------------------------------
  // SENSOR TANAH DAN VALVE
  // ---------------------------------------------------

  if (
      sekarang -
      waktuSoilTerakhir >=
      INTERVAL_SOIL
  ) {
    waktuSoilTerakhir =
        sekarang;

    bacaSensorKelembapan();

    kontrolValve(
        sekarang
    );
  }

  // Proteksi valve maksimum
  if (
      valveTerbuka &&
      sekarang -
      waktuValveMulai >=
      MAKSIMUM_WAKTU_VALVE
  ) {
    setValve(false);

    waktuValveBerhenti =
        sekarang;

    valveTimeout =
        true;
  }

  // ---------------------------------------------------
  // NPK MODBUS
  // ---------------------------------------------------

  if (
      sekarang -
      waktuNpkTerakhir >=
      INTERVAL_NPK
  ) {
    waktuNpkTerakhir =
        sekarang;

    bacaSensorNpk();
  }

  // ---------------------------------------------------
  // KIRIM UDP
  // ---------------------------------------------------

  if (
      sekarang -
      waktuUdpTerakhir >=
      INTERVAL_UDP
  ) {
    waktuUdpTerakhir =
        sekarang;

    kirimDataUdp();
  }
 if (
      sekarang -
      waktuThingSpeakTerakhir >=
      INTERVAL_THINGSPEAK
  ) {
    waktuThingSpeakTerakhir =
        sekarang;

    kirimDataThingSpeak();
  }

  // ---------------------------------------------------
  // SERIAL MONITOR
  // ---------------------------------------------------

  if (
      sekarang -
      waktuTampilTerakhir >=
      INTERVAL_TAMPIL
  ) {
    waktuTampilTerakhir =
        sekarang;

    tampilkanDataSerial();
  }

  delay(2);
}
