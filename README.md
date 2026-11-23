# Smart IoT Medicine Box

Kotak obat pintar dengan 3 kotak: **Pagi**, **Siang**, dan **Malam**. Saat waktunya minum obat, kotak ini berbunyi, menampilkan nama obat di layar, dan mengirim pesan ke Telegram. Kotak tahu obat sudah diminum ketika tutup kotak yang benar dibuka.

Bagian pertama dokumen ini untuk pemakai sehari-hari. Bagian **Untuk developer** di bawah berisi cara build dan flash firmware.

---

# Panduan pemakaian

## Isi kotak

| Bagian | Fungsi |
|---|---|
| Kotak 1, 2, 3 | Tempat obat Pagi, Siang, dan Malam |
| Layar kecil (OLED) | Menampilkan tanggal, jam, dan jadwal obat berikutnya |
| Buzzer | Berbunyi saat waktunya minum obat |
| Kabel USB-C | Sumber listrik. Pakai charger HP biasa (5V) |

## Yang perlu disiapkan

- HP dengan aplikasi **Telegram**.
- WiFi rumah **2.4 GHz**. Kotak ini tidak bisa memakai WiFi 5 GHz. Kalau nama WiFi rumah ada dua (misalnya `Rumah` dan `Rumah-5G`), pilih yang tanpa `5G`.
- Charger HP dan kabel USB-C.

## Langkah 1: Buat bot Telegram

Bot adalah "akun" Telegram milik kotak obat. Lewat bot ini kotak mengirim pengingat, dan Anda mengatur jadwal.

1. Buka Telegram, cari **@BotFather**, lalu tekan **Start**.
2. Kirim pesan `/newbot`.
3. Ketik nama bot, misalnya `Kotak Obat Ibu`.
4. Ketik username bot. Harus diakhiri kata `bot`, misalnya `kotakobatibu_bot`.
5. BotFather membalas dengan **token**, contohnya `123456789:AAHxxxxxxxxxxxx`. Salin dan simpan token ini. Jangan bagikan ke orang lain.
6. Buka bot baru Anda (klik link `t.me/...` dari BotFather) lalu tekan **Start**. Langkah ini wajib. Tanpa ini, bot tidak bisa mengirim pesan ke Anda.

## Langkah 2: Cari chat ID

Chat ID adalah nomor akun Telegram Anda. Kotak obat hanya mau menerima perintah dan mengirim pesan ke chat ID yang didaftarkan.

1. Di Telegram, cari **@userinfobot** dan tekan **Start**.
2. Bot itu membalas dengan `Id: 123456789`. Angka tersebut adalah chat ID Anda.

Beberapa anggota keluarga bisa menerima pengingat. Setiap orang mencari chat ID-nya sendiri dengan cara yang sama, dan masing-masing harus menekan **Start** di bot kotak obat.

Untuk grup keluarga: masukkan bot ke grup. Chat ID grup berupa angka negatif yang diawali `-100`, misalnya `-1001234567890`. Lihat bagian **Untuk developer** untuk cara mencarinya.

## Langkah 3: Sambungkan kotak ke WiFi

1. Colokkan kotak ke charger. Layar menampilkan **Setup WiFi**.
2. Di HP, buka pengaturan WiFi dan sambungkan ke WiFi **MedicineBox**. Passwordnya `obat1234`.
3. Halaman pengaturan biasanya terbuka sendiri. Kalau tidak:
   - matikan data seluler sementara,
   - kalau HP bertanya "WiFi ini tidak ada internet, tetap tersambung?", pilih **Ya**,
   - buka browser dan ketik `http://192.168.4.1`.
4. Tekan **Configure WiFi**.
5. Pilih WiFi rumah, lalu isi password WiFi.
6. Isi **Bot token** dengan token dari Langkah 1.
7. Isi **Chat ID** dengan nomor dari Langkah 2. Kalau lebih dari satu, pisahkan dengan koma tanpa spasi, misalnya `123456789,987654321`.
8. Tekan **Save**.

Dalam beberapa detik, layar menampilkan **Terhubung**, dan bot mengirim pesan "Medicine Box menyala" ke Telegram. Kotak siap dipakai.

Pengaturan ini tersimpan. Kalau listrik mati lalu menyala lagi, kotak tersambung sendiri tanpa perlu setup ulang.

## Langkah 4: Atur jadwal obat

Atur jadwal dengan mengirim pesan ke bot. Formatnya:

```
/set <nomor kotak> <jam> <nama obat>
```

Nomor kotak: **1 = Pagi**, **2 = Siang**, **3 = Malam**. Jam ditulis dengan format 24 jam (`07:00`, `19:30`).

Contoh:
```
/set 1 07:00 Amlodipin 5mg
/set 2 12:30 Metformin
/set 3 19:00 Vitamin D
```

Kirim `/jadwal` untuk memeriksa hasilnya.

## Sehari-hari: saat waktunya minum obat

1. **Waktu minum tiba.** Buzzer berbunyi *beep beep beep*, diam 3 detik, lalu berulang. Layar menampilkan nama obat. Telegram menerima pesan dengan dua tombol: **Sudah minum** dan **Tunda 10 menit**.
2. **Buka tutup kotak yang sesuai** dan ambil obatnya. Buzzer langsung berhenti, obat tercatat **sudah diminum**, dan Telegram menerima konfirmasi.
3. **Belum sempat minum?** Tekan **Tunda 10 menit** di Telegram. Buzzer diam, lalu berbunyi lagi 10 menit kemudian.
4. **Sudah minum tapi tidak lewat kotak?** Tekan **Sudah minum** di Telegram.
5. **Lewat 30 menit tanpa diminum?** Buzzer berhenti, obat tercatat **terlewat**, dan Telegram menerima peringatan. Keluarga yang terdaftar ikut tahu.

Penting: kalau tutup kotak sudah terbuka saat buzzer berbunyi, tutup dulu, lalu buka lagi. Kotak hanya menghitung gerakan membuka.

## Layar

Saat tidak ada pengingat, layar menampilkan:
```
   Sen 23/11/2026          <- hari dan tanggal
       14:30               <- jam
Next Siang 12:30 Metformin <- jadwal berikutnya
```
Setelah 30 detik tanpa aktivitas, layar meredup tetapi tetap menyala. Layar kembali terang saat tutup kotak dibuka atau ditutup, atau saat waktunya minum obat.

Kalau layar menampilkan `--:--` atau "Menunggu sinkron waktu", kotak belum mendapat jam dari internet. Pastikan WiFi menyala. Jadwal baru berjalan setelah jam muncul.

## Daftar perintah Telegram

| Ketik | Fungsi |
|---|---|
| `/bantuan` | Menampilkan daftar perintah |
| `/jadwal` | Melihat semua jadwal |
| `/set 1 07:00 Nama Obat` | Mengatur jadwal kotak 1, 2, atau 3 (nama obat maksimal 24 huruf) |
| `/hapus 2` | Mematikan jadwal kotak 2 |
| `/status` | Melihat tutup kotak terbuka atau tertutup, jadwal, dan kapan terakhir diminum |
| `/riwayat` | Riwayat minum obat 7 hari terakhir |
| `/tunda 15` | Menunda pengingat yang sedang berbunyi selama 15 menit (1–120, tanpa angka = 10 menit) |
| `/diam` | Mematikan buzzer selama 1 jam (pesan Telegram tetap terkirim) |
| `/tes` | Mengetes buzzer dan layar |
| `/setup` | Membuka halaman pengaturan selama 3 menit, untuk mengganti token atau chat ID |
| `/resetwifi` | Menghapus WiFi tersimpan, lalu kotak membuka halaman pengaturan. Jadwal dan riwayat tetap aman |

## Ganti WiFi (pindah rumah atau ganti password router)

- **Kalau kotak tidak bisa tersambung ke WiFi lama:** tunggu 1 menit. Layar menampilkan **Setup WiFi**. Ulangi Langkah 3.
- **Kalau kotak masih tersambung:** kirim `/resetwifi` ke bot, lalu ulangi Langkah 3.

Halaman pengaturan menutup sendiri setelah 3 menit kalau tidak ada yang disimpan. Selama itu, pengingat obat tetap berjalan.

## Kalau ada masalah

| Masalah | Yang bisa dicoba |
|---|---|
| Bot tidak membalas | Pastikan layar tidak menampilkan **Setup WiFi**. Pastikan chat ID Anda sudah didaftarkan di halaman pengaturan. Kirim `/setup` dari HP lain yang terdaftar untuk memeriksa ulang token dan chat ID |
| Satu anggota keluarga tidak menerima pesan | Orang tersebut belum menekan **Start** di bot, atau chat ID-nya salah |
| WiFi **MedicineBox** tidak muncul | Kotak sudah tersambung ke WiFi. Kirim `/setup` atau `/resetwifi` untuk membuka pengaturan |
| Halaman pengaturan tidak terbuka | Matikan data seluler, lalu buka `http://192.168.4.1` di browser |
| Buzzer tidak berhenti walau kotak dibuka | Pastikan kotak yang dibuka sesuai jadwal (Pagi/Siang/Malam). Tutup dulu, lalu buka lagi. Atau tekan **Sudah minum** di Telegram |
| Jadwal tidak berbunyi | Kirim `/jadwal` dan periksa jamnya. Pastikan layar menampilkan jam, bukan `--:--` |
| Buzzer mengganggu sementara | Kirim `/diam` |

## Yang perlu diketahui

- Waktu yang dipakai adalah **WIB**.
- Kalau internet putus, buzzer dan layar tetap bekerja sesuai jadwal. Pesan Telegram disimpan (maksimal 10 pesan) dan dikirim setelah internet kembali.
- Kalau listrik mati, jadwal, riwayat, dan pengaturan WiFi tetap tersimpan. Pesan Telegram yang belum terkirim akan hilang.
- Pesan dari akun Telegram yang tidak terdaftar diabaikan.

---

# Untuk developer

Board: ESP32-C3 Super Mini. Wiring: [WIRING.md](WIRING.md). Checklist uji: [TESTING.md](TESTING.md). Tes per komponen: [hardware_test/README.md](hardware_test/README.md).

## Build dan flash

Butuh PlatformIO (VS Code extension atau CLI).
```bash
pio test -e native          # unit test logika
pio run -e c3 -t upload     # flash lewat USB-C
pio device monitor          # log serial 115200
```
Jika upload gagal: tahan tombol BOOT, tekan RST, lepas BOOT, lalu upload ulang.

## Konfigurasi

- WiFi, bot token, dan chat ID diisi lewat portal WiFiManager dan disimpan di flash.
- `include/secrets.h` berisi nilai awal untuk flash pertama. Di repo, isinya kosong. Kalau diisi untuk pengembangan, jalankan `git update-index --skip-worktree include/secrets.h` agar kredensial tidak ikut ter-commit.
- Nama dan password hotspot setup: `SETUP_AP_NAME`, `SETUP_AP_PASS` di `include/config.h`.
- Pola buzzer, waktu redup layar, batas 30 menit, dan interval pengingat juga diatur di `include/config.h`.
- Buzzer pasif: ubah `BUZZER_ACTIVE` menjadi `false` di `include/config.h`.

## Cari chat ID grup

1. Masukkan bot ke grup, lalu kirim `/start@NamaBot` di grup.
2. Cabut listrik kotak obat. Saat menyala, kotak mengambil pesan masuk lebih dulu sehingga hasil di browser kosong.
3. Buka `https://api.telegram.org/bot<TOKEN>/getUpdates` dan ambil angka di `"chat":{"id": ...}`.

## Alur pengingat

1. Jam jadwal tiba: buzzer bunyi beep 3x, jeda 3 detik (`ALERT_PAUSE_MS`), berulang sampai diminum, ditunda, di-`/diam`, atau terlewat.
2. Pintu dibuka dalam 30 menit (`WINDOW_MIN`): tercatat diminum.
3. Pengingat Telegram diulang tiap 5 menit (`REMIND_EVERY_MIN`, maks. `MAX_REMINDERS` kali). Setelah 30 menit tercatat terlewat. Tunda memperpanjang batas ini.

## Struktur

```
include/        config.h pins.h secrets.h secrets.example.h
lib/core/       logika tanpa hardware (scheduler, debounce, history, commands, backoff)
test/           unit test native untuk lib/core
src/            main.cpp app.h buzzer display net settings storage telegram_handler
hardware_test/  sketch uji per komponen (env hw_*)
```
