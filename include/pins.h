#pragma once
// ESP32-C3 Super Mini. Strapping pins: 2, 8, 9.
// GPIO8/9 are the board's default I2C; OLED pull-ups keep them HIGH at boot (safe).
constexpr int PIN_SDA = 8;
constexpr int PIN_SCL = 9;
constexpr int PIN_REED[3] = {3, 4, 5};  // Pagi, Siang, Malam
constexpr int PIN_REED_SPARE = 6;
constexpr int PIN_BUZZER = 7;            // via NPN transistor
