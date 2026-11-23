# Wiring — Smart IoT Medicine Box

Board: **ESP32-C3 Super Mini**. Semua nomor pin di dokumen ini adalah nomor **GPIO**, sama dengan yang dipakai firmware (`include/pins.h`).

## Letak pin di board

Posisi board: port USB-C di atas, chip menghadap ke kamu.

```
                 +----[ USB-C ]----+
        GPIO5  o |                 | o  5V
        GPIO6  o |                 | o  GND
        GPIO7  o |  [BOOT]  [RST]  | o  3V3
        GPIO8  o |                 | o  GPIO4
        GPIO9  o |     ESP32-C3    | o  GPIO3
        GPIO10 o |                 | o  GPIO2
        GPIO20 o |                 | o  GPIO1
        GPIO21 o |                 | o  GPIO0
                 +-----------------+
```

Pin kiri dari atas ke bawah: GPIO5, 6, 7, 8, 9, 10, 20, 21.
Pin kanan dari atas ke bawah: 5V, GND, 3V3, GPIO4, 3, 2, 1, 0.

**Abaikan label SDA/SCL/SCK/MISO di gambar pinout penjual.** Beberapa gambar menulis SDA di GPIO6 dan SCL di GPIO7. Firmware ini memakai pin sendiri: OLED di **GPIO8 (SDA)** dan **GPIO9 (SCL)**. Ikuti nomor GPIO di tabel bawah, bukan label fungsinya.

## Tabel sambungan

| Komponen | Kaki komponen | Sambung ke | Posisi di board |
|---|---|---|---|
| OLED SSD1306 | VCC | 3V3 | kanan, pin ke-3 |
| OLED | GND | GND | kanan, pin ke-2 |
| OLED | SDA | GPIO8 | kiri, pin ke-4 |
| OLED | SCL (kadang tertulis SCK) | GPIO9 | kiri, pin ke-5 |
| Reed Pagi (kotak 1) | kaki 1 / kaki 2 | GPIO3 / GND | kanan, pin ke-5 |
| Reed Siang (kotak 2) | kaki 1 / kaki 2 | GPIO4 / GND | kanan, pin ke-4 |
| Reed Malam (kotak 3) | kaki 1 / kaki 2 | GPIO5 / GND | kiri, pin ke-1 |
| Reed cadangan (opsional) | kaki 1 / kaki 2 | GPIO6 / GND | kiri, pin ke-2 |
| Buzzer | (+) kaki panjang | GPIO7 | kiri, pin ke-3 |
| Buzzer | (−) kaki pendek | GND | kanan, pin ke-2 |

Pin yang tidak dipakai: GPIO0, 1, 2, 10, 20, 21, dan 5V.

## Diagram

```
                       ESP32-C3 Super Mini
                     +---------------------+
 reed Malam ---------| GPIO5           5V  |  (tidak dipakai)
 reed cadangan ------| GPIO6           GND |------+---- GND rail
 BUZZER (+) ---------| GPIO7           3V3 |---- OLED VCC
 OLED SDA -----------| GPIO8         GPIO4 |---- reed Siang
 OLED SCL -----------| GPIO9         GPIO3 |---- reed Pagi
                     | GPIO10        GPIO2 |
                     | GPIO20        GPIO1 |
                     | GPIO21        GPIO0 |
                     +---------------------+

 GND rail (breadboard) ---+--- OLED GND
                          +--- BUZZER (−)
                          +--- kaki 2 reed Pagi
                          +--- kaki 2 reed Siang
                          +--- kaki 2 reed Malam
                          +--- kaki 2 reed cadangan
```

## Catatan

- **Board hanya punya satu pin GND.** Sambungkan pin GND ke jalur minus breadboard, lalu semua kabel GND komponen (OLED, buzzer, keempat reed) ke jalur itu.
- **Reed switch** 2 kaki tidak punya polaritas, jadi kaki mana pun boleh ke GPIO. Tidak perlu resistor, karena firmware memakai pull-up internal (`INPUT_PULLUP`). Tutup kotak tertutup (magnet dekat) = pin LOW. Tutup terbuka = pin HIGH.
- **OLED** memakai 3V3, bukan 5V.
- **Buzzer** memakai buzzer aktif (berbunyi sendiri saat diberi tegangan). Kalau memakai buzzer pasif, ubah `BUZZER_ACTIVE` menjadi `false` di `include/config.h`.
- **GPIO8 dan GPIO9** adalah strapping pin. Pull-up di modul OLED menjaga keduanya HIGH saat boot, jadi board menyala normal. LED biru di board ikut berkedip saat layar diperbarui. Ini normal.
- Cek wiring per komponen dengan sketch di `hardware_test/` sebelum memasang ke casing. Lihat [hardware_test/README.md](hardware_test/README.md).

## Penempatan di 3 box casing

- **Box 1 / 2 / 3 = kompartemen Pagi / Siang / Malam.** Satu reed per box.
- **Reed switch** ditempel di bibir badan box (bagian tetap), dekat sisi engsel yang berlawanan. Kabel keluar lewat lubang kecil di belakang.
- **Magnet** ditempel di tutup, tepat di atas reed saat tutup tertutup. Jarak ideal di bawah 10 mm. Uji dulu dengan Serial/`/status` sebelum lem permanen.
- **ESP32 + breadboard + buzzer** taruh di box tengah (Siang) di bawah sekat, atau di box ke-4 kecil di belakang. Buzzer dekat lubang agar suara keluar.
- **OLED** di dinding depan box tengah dengan potongan jendela 26×15 mm.
- Ikat kabel dan beri label 1/2/3 agar tidak tertukar. USB-C diarahkan ke belakang.
