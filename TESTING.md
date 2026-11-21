# Checklist Uji

Jalankan `pio device monitor` selama pengujian. Gunakan jadwal 2 menit dari sekarang: `/set 1 HH:MM Tes`.

## Unit test (laptop)
- [ ] `pio test -e native` → semua PASS

## OLED
- [ ] Saat boot tampil "Medicine Box / Memulai..."
- [ ] Layar idle: status WiFi, tanggal, jam besar, jadwal berikutnya
- [ ] Redup setelah 30 detik, mati setelah 2 menit
- [ ] Membuka pintu mana pun menyalakan layar lagi
- [ ] Layar kosong: cek SDA/SCK tertukar, VCC ke 3V3, alamat 0x3C/0x3D

## Buzzer
- [ ] `/tes` → bunyi 0,5 detik + OLED "Tes OLED"
- [ ] Saat alarm: pola 3 beep pendek, jeda 1 detik
- [ ] `/diam` saat alarm → buzzer berhenti

## Reed switch (ulangi untuk slot 1, 2, 3)
- [ ] Tutup tertutup → `/status` "Pintu: tertutup"
- [ ] Tutup terbuka → `/status` "Pintu: terbuka"
- [ ] Buka-tutup cepat tidak memicu catatan ganda
- [ ] Slot tertukar: cek label kabel GPIO3/4/5

## Jadwal
- [ ] Jam tiba → buzzer + OLED + Telegram dengan 2 tombol
- [ ] Buka pintu dalam 30 menit → buzzer berhenti, "sudah diminum", tercatat di `/riwayat`
- [ ] Tombol "Sudah minum" → sama seperti buka pintu
- [ ] Tombol "Tunda 10 menit" / `/tunda 10` → pengingat berikutnya 10 menit kemudian
- [ ] Tidak dibuka → pengingat ke-1/2/3 di menit 5/10/15, "terlewat" di menit 30
- [ ] Pintu sudah terbuka saat alarm → tidak dihitung sampai ditutup dan dibuka lagi
- [ ] `/set` format salah (`/set 4 07:00 X`, `/set 1 25:00 X`) → pesan "Format salah"

## Mati listrik
- [ ] Atur jadwal, cabut USB, colok lagi → `/jadwal` tetap sama
- [ ] `/riwayat` tetap berisi data sebelum dicabut
- [ ] Cabut-colok setelah jadwal tercatat → slot tidak bunyi lagi di hari yang sama

## WiFi putus
- [ ] Matikan router setelah waktu sinkron → OLED "WiFi: --", jam tetap jalan
- [ ] Jadwal tiba saat offline → buzzer + OLED tetap bunyi, catatan tersimpan
- [ ] Nyalakan router → board tersambung ulang dalam ±10 detik, notifikasi tertunda terkirim

## Keamanan
- [ ] Kirim perintah dari akun di luar `ALLOWED_CHATS` → tidak ada balasan, jadwal tidak berubah
- [ ] `git status` tidak menampilkan `include/secrets.h`
