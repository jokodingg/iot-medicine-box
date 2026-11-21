# Smart IoT Medicine Box

Kotak obat 3 kompartemen (Pagi/Siang/Malam) berbasis ESP32-C3 Super Mini. Pengingat lewat buzzer, OLED, dan Telegram. Reed switch mendeteksi pintu dibuka. Riwayat 7 hari tersimpan di flash. Tetap berjalan saat WiFi mati.

Wiring: lihat [WIRING.md](WIRING.md). Checklist uji: [TESTING.md](TESTING.md).

## 1. Buat bot Telegram
1. Buka Telegram, cari **@BotFather**, kirim `/newbot`.
2. Isi nama dan username bot (harus diakhiri `bot`).
3. Salin token, contoh `123456789:AAH...`.

## 2. Cari chat ID
1. Kirim pesan apa saja ke bot baru Anda.
2. Buka `https://api.telegram.org/bot<TOKEN>/getUpdates` di browser.
3. Ambil angka di `"chat":{"id": ...}`. Alternatif: chat **@userinfobot**.
4. Untuk grup keluarga: tambahkan bot ke grup, kirim pesan, ID grup diawali `-100`.

## 3. Isi kredensial
```bash
cp include/secrets.example.h include/secrets.h
```
Edit `include/secrets.h`: `WIFI_SSID`, `WIFI_PASS`, `BOT_TOKEN`, `ALLOWED_CHATS` (pisahkan dengan koma). File ini tidak ikut git. WiFi harus 2.4 GHz.

## 4. Build dan flash
Butuh PlatformIO (VS Code extension atau CLI).
```bash
pio test -e native          # unit test logika
pio run -e c3 -t upload     # flash lewat USB-C
pio device monitor          # log serial 115200
```
Jika upload gagal: tahan tombol BOOT, tekan RST, lepas BOOT, lalu upload ulang.

## 5. Pemakaian
Atur jadwal dari Telegram:
```
/set 1 07:00 Amlodipin 5mg
/set 2 12:30 Metformin
/set 3 19:00 Vitamin D
```

| Perintah | Fungsi |
|---|---|
| `/start`, `/bantuan` | daftar perintah |
| `/status` | pintu, jadwal, terakhir diminum per kompartemen |
| `/jadwal` | semua jadwal |
| `/set <1-3> <HH:MM> <nama>` | atur jadwal (nama maks. 24 karakter) |
| `/hapus <1-3>` | nonaktifkan slot |
| `/riwayat` | riwayat 7 hari |
| `/tunda <menit>` | tunda pengingat aktif (default 10, maks. 120) |
| `/diam` | matikan buzzer 1 jam |
| `/tes` | tes buzzer dan OLED |

Alur pengingat:
1. Jam jadwal tiba: buzzer bunyi, OLED tampil nama obat, Telegram kirim notifikasi dengan tombol **Sudah minum** dan **Tunda 10 menit**.
2. Buka pintu kompartemen dalam 30 menit: tercatat **diminum**, buzzer berhenti, konfirmasi dikirim.
3. Belum dibuka: pengingat ulang tiap 5 menit (maks. 3x). Setelah 30 menit tercatat **terlewat** dan peringatan dikirim.

Catatan:
- Pintu yang sudah terbuka saat alarm harus ditutup lalu dibuka lagi agar terhitung.
- Waktu memakai WIB (UTC+7) dari NTP. Sebelum waktu tersinkron, jadwal tidak berjalan. Setelah sinkron, jadwal tetap berjalan tanpa WiFi selama board menyala.
- Saat offline, notifikasi disimpan di antrean (maks. 10) dan dikirim setelah online. Antrean hilang jika listrik mati; riwayat tetap tersimpan.
- Chat ID di luar whitelist diabaikan tanpa balasan.
- Ganti ke buzzer pasif: ubah `BUZZER_ACTIVE` menjadi `false` di `include/config.h`.

## Struktur
```
include/   config.h pins.h secrets.example.h
lib/core/  logika tanpa hardware (scheduler, debounce, history, commands) + unit test di test/
src/       main.cpp app.h buzzer display net storage telegram_handler
```
