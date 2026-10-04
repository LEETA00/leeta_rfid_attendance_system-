# Leeta: ESP32 RFID Attendance System

An RFID attendance system. An ESP32 reads cards with an MFRC522, shows the result on a 16x2 LCD, and sends each scan to a Flask server that logs to SQLite and serves a live web dashboard.

> This repository is a clean rewrite of a project I built after my final defense, published as a reference implementation. Check pin numbers against your own wiring.

## Features
- Card scan with LCD feedback and buzzer (short beep = saved, long beep = unknown card or error)
- Live dashboard: present today, registered users, scans today, ESP32 online status, live feed, scans per hour
- Add and remove users and cards from the browser

## Hardware
ESP32 DevKit, MFRC522 RFID reader, 16x2 I2C LCD, active buzzer, breadboard and jumper wires.

| MFRC522 | ESP32 | LCD / Buzzer | ESP32 |
|---|---|---|---|
| SDA (SS) | GPIO 5 | LCD SDA | GPIO 21 |
| SCK | GPIO 18 | LCD SCL | GPIO 22 |
| MOSI | GPIO 23 | Buzzer + | GPIO 15 |
| MISO | GPIO 19 | | |
| RST | GPIO 4 | | |
| 3.3V / GND | 3V3 / GND | | |

The MFRC522 is a 3.3V device. Do not power it from 5V.

## Setup

**Server** (PC on the same WiFi as the ESP32):

    cd server
    pip install -r requirements.txt
    LEETA_API_KEY=your-secret python app.py

Open `http://<pc-ip>:5000` in a browser.

**Firmware:** install the `MFRC522` and `LiquidCrystal_I2C` libraries in Arduino IDE. Set `WIFI_SSID`, `WIFI_PASS`, `SERVER`, and `API_KEY` in `firmware/leeta_esp32/leeta_esp32.ino`, then upload.

## How it works
1. Card tapped, ESP32 reads the UID.
2. ESP32 sends the UID to `/api/scan` with an `X-API-Key` header.
3. Server logs the scan, matches the UID to a user, and replies known or unknown.
4. The dashboard refreshes from `/api/stats` every 3 seconds.

## Roadmap
CSV export, attendance reports per user, HTTPS, per-day present/absent view.