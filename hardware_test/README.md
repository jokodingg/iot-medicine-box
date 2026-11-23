# Hardware test

One sketch per component. Each sketch runs alone, without the main firmware.
Pins come from `include/pins.h`, so the tests use the same wiring as `WIRING.md`.

Run one test:

```
pio run -e hw_reed -t upload
pio device monitor
```

Upload the main firmware again after testing: `pio run -e c3 -t upload`.

| Env | Component | Pass when |
|---|---|---|
| `hw_i2c_scan` | I2C bus | Serial shows `device at 0x3C <- OLED` |
| `hw_oled` | OLED SSD1306 | Text and frame are visible, counter goes up |
| `hw_reed` | 4 reed switches | Opening a lid prints `BUKA`, closing prints `TUTUP`, for the correct box |
| `hw_buzzer` | Buzzer + NPN | Active buzzer beeps in step 1; passive buzzer plays rising pitches in step 2 |
| `hw_oled_wifi` | OLED under WiFi load | Screen keeps updating after `WIFI ON` (15 s) and after `CONNECTED` |
| `hw_wifi_telegram` | WiFi, NTP, Telegram | IP and WIB time print, every chat in `ALLOWED_CHATS` gets a message |

Suggested order: `hw_i2c_scan`, `hw_oled`, `hw_reed`, `hw_buzzer`, `hw_wifi_telegram`.

`hw_wifi_telegram` needs `include/secrets.h` (copy from `secrets.example.h`).
