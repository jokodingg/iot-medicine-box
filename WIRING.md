# Wiring — Smart IoT Medicine Box

Board: ESP32-C3 Super Mini. Usable pins: GPIO0–10, 20, 21. Strapping pins: 2, 8, 9.

## Pin table

| Komponen | Pin komponen | ESP32-C3 | Catatan |
|---|---|---|---|
| OLED SSD1306 | GND | GND | |
| OLED | VCC | 3V3 | Jangan pakai 5V jika modul tanpa regulator |
| OLED | SCK (SCL) | GPIO9 | I2C default board; pull-up modul membuat boot normal |
| OLED | SDA | GPIO8 | I2C default board; LED onboard ikut berkedip, normal |
| Reed Pagi | kaki 1 / kaki 2 | GPIO3 / GND | INPUT_PULLUP, tanpa resistor |
| Reed Siang | kaki 1 / kaki 2 | GPIO4 / GND | |
| Reed Malam | kaki 1 / kaki 2 | GPIO5 / GND | |
| Reed cadangan | kaki 1 / kaki 2 | GPIO6 / GND | belum dipakai firmware |
| Buzzer (+) | kaki panjang | 3V3 | |
| Buzzer (−) | kaki pendek | Collector NPN | |
| NPN S8050/2N2222 | Base | GPIO7 lewat resistor 1kΩ | |
| NPN | Emitter | GND | |

Opsional: dioda 1N4148 paralel buzzer (katoda ke 3V3) jika buzzer magnetik. Untuk buzzer piezo tidak wajib.

Reed switch 2 kaki tidak punya polaritas. Pintu tertutup = magnet dekat = kontak tertutup = pin LOW. Pintu terbuka = pin HIGH.

## Diagram

```
                 ESP32-C3 Super Mini
               +---------------------+
   OLED VCC ---| 3V3            GPIO9|--- OLED SCK
   OLED GND ---| GND            GPIO8|--- OLED SDA
               |                GPIO3|---[reed Pagi ]--- GND
               |                GPIO4|---[reed Siang]--- GND
               |                GPIO5|---[reed Malam]--- GND
               |                GPIO6|---[reed spare]--- GND
               |                GPIO7|---[1kΩ]---+
               +---------------------+           |
                                                 B
   3V3 ---(+)BUZZER(-)--------------------- C  NPN
                                                 E
                                                 |
                                                GND
```

Semua GND disatukan di rail breadboard.

## Penempatan di 3 box casing

- **Box 1 / 2 / 3 = kompartemen Pagi / Siang / Malam.** Satu reed per box.
- **Reed switch** ditempel di bibir badan box (bagian tetap), dekat sisi engsel yang berlawanan. Kabel keluar lewat lubang kecil di belakang.
- **Magnet** ditempel di tutup, tepat di atas reed saat tutup tertutup. Jarak ideal di bawah 10 mm. Uji dulu dengan Serial/`/status` sebelum lem permanen.
- **ESP32 + breadboard + buzzer** taruh di box tengah (Siang) di bawah sekat, atau di box ke-4 kecil di belakang. Buzzer dekat lubang agar suara keluar.
- **OLED** di dinding depan box tengah dengan potongan jendela 26×15 mm.
- Ikat kabel dan beri label 1/2/3 agar tidak tertukar. USB-C diarahkan ke belakang.
