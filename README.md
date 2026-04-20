# 🚪 IoT-Based Smart Door Control System

**Course:** CS-744 — Internet of Things  
**Programme:** Dual Degree (B.Tech + M.Tech)  
**Institute:** National Institute of Technology Hamirpur  
**Author:** Piyush (22DCS014)  
**Guide:** Dr. Robin Singh Bhadoria, Assistant Professor, Dept. of CSE  
**Year:** 2026

---

## 📌 Project Overview

This project implements a **real-time, IoT-enabled Smart Door Control System** built on the **ESP32 microcontroller**. It combines RFID-based contactless authentication, PIR motion detection, servo-driven physical locking, and cloud-based remote monitoring into a single, cohesive embedded security platform.

All access events are logged to **ThingSpeak** (MathWorks) over WiFi in real time, giving remote administrators full visibility into door state, access history, and motion alerts from any internet-connected device.

---

## 🧰 Hardware Components

| Component | Role | Interface |
|-----------|------|-----------|
| ESP32 DevKit V1 | Central MCU + WiFi | All peripherals |
| MFRC522 RFID Reader + Cards | Contactless UID authentication | SPI (GPIO 5/4/18/19/23) |
| HC-SR501 PIR Sensor | Motion detection | Digital (GPIO 33) |
| SG90 Servo Motor | Physical door lock (0°/90°) | PWM (GPIO 13) |
| I2C LCD 16×2 | Local status display | I2C 0x27 (GPIO 21/22) |
| Green LED | Access Granted indicator | GPIO 26 |
| Red LED | Access Denied / Motion Alert | GPIO 27 |
| Blue LED | WiFi / Cloud activity | GPIO 25 |
| Active Buzzer | Audio access feedback | GPIO 32 |

**Estimated Hardware Cost:** ≈ ₹2,800

---

## 🗂️ Repository Structure

```
.
├── sketch.ino                  # Main Arduino firmware (ESP32)
├── populate_thingspeak.py      # Python script to populate ThingSpeak with demo data
├── diagram.json                # Wokwi circuit diagram
├── wokwi.toml                  # Wokwi simulation config
├── libraries.txt               # Required Arduino libraries list
├── components_explained.md     # Detailed component descriptions
├── smart_door_report.tex       # Full project report (LaTeX, IEEE format)
├── water_quality_report.tex    # Reference report (LaTeX)
└── README.md                   # This file
```

---

## ⚙️ System Architecture

```
RFID Card Scan / PIR Motion
         │
         ▼
   ESP32 processes event
         │
    ┌────┴────┐
    │  Auth?  │
    └────┬────┘
    YES  │  NO
    ▼         ▼
Servo 90°   Servo stays 0°
Green LED   Red LED (3 beeps)
2 beeps     LCD: ACCESS DENIED
LCD: GRANTED
    │
    ▼ (after 5 seconds)
Servo auto-locks → 0°
    │
    ▼ (every 15s + on every card scan)
ThingSpeak Cloud
├── Field 1: Door Status  (0=Closed, 1=Open)
├── Field 2: Auth Result  (0=Denied, 1=Granted)
├── Field 3: Motion Alert (0=Clear,  1=Detected)
└── Field 4: Total Attempts (running count)
```

---

## 🔑 Key Features

- **Contactless RFID Authentication** — 3-card authorised UID table with byte-level comparison
- **Servo Auto-Lock** — Door automatically re-locks after 5 seconds using non-blocking `millis()` timer
- **PIR Pre-Entry Detection** — Detects approaching persons before card presentation
- **Multi-Modal Feedback** — Distinct LED colors + buzzer beep patterns for granted vs. denied
- **Dual-Trigger Cloud Upload** — Periodic (15s) + immediate (on every card scan) ThingSpeak logging
- **Non-Blocking Firmware** — All timing via `millis()`; loop never freezes during alerts

---

## ☁️ ThingSpeak Cloud Integration

| Field | Parameter | Values |
|-------|-----------|--------|
| Field 1 | Door Status | 1 = Open, 0 = Closed |
| Field 2 | Auth Result | 1 = Granted, 0 = Denied |
| Field 3 | Motion Alert | 1 = Detected, 0 = Clear |
| Field 4 | Total Attempts | Running count of card scans |

> **API Key:** Configured in `sketch.ino` and `populate_thingspeak.py`

---

## 🖥️ Simulation

This project is designed for the **[Wokwi](https://wokwi.com)** online simulator.

1. Open [wokwi.com](https://wokwi.com)
2. Import `diagram.json` and `sketch.ino`
3. Start the simulation and scan virtual RFID cards
4. Monitor live data on your ThingSpeak channel

---

## 🐍 Populating ThingSpeak with Demo Data

To fill your ThingSpeak channel with realistic demo data without running the simulation:

```bash
pip install requests
python populate_thingspeak.py
```

This sends **55 pre-defined events** (authorised entries, denied attempts, motion alerts, and idle heartbeats) spaced 16 seconds apart.

---

## 📦 Required Arduino Libraries

See `libraries.txt` for the full list. Install via Arduino IDE Library Manager:

- `ESP32Servo`
- `HTTPClient` (built-in with ESP32 core)
- `LiquidCrystal_I2C`
- `MFRC522`
- `WiFi` (built-in with ESP32 core)
- `Wire` (built-in)

---

## 📄 Report

The full IEEE-format project report is available in `smart_door_report.tex`.  
Compile with:
```
pdflatex smart_door_report.tex   # Run 3 times for TOC and references
```
Or upload directly to [Overleaf](https://www.overleaf.com).

---

## ✅ Test Results

All **12 functional test cases** passed on the Wokwi simulation platform:

| # | Test | Result |
|---|------|--------|
| 1 | Authorised card scan | ✅ Pass |
| 2 | Unauthorised card scan | ✅ Pass |
| 3 | Auto-lock after 5s | ✅ Pass |
| 4 | PIR motion detection | ✅ Pass |
| 5 | PIR cleared | ✅ Pass |
| 6 | Multi-card scenario | ✅ Pass |
| 7 | Immediate cloud push on scan | ✅ Pass |
| 8 | LCD door status display | ✅ Pass |
| 9 | Non-blocking buzzer | ✅ Pass |
| 10 | ThingSpeak periodic upload | ✅ Pass |
| 11 | WiFi disconnect handling | ✅ Pass |
| 12 | Attempt counter accuracy | ✅ Pass |

**Pass Rate: 12/12 (100%)**

---

## 📬 Contact

**Piyush** — 22DCS014  
Department of Computer Science and Engineering  
National Institute of Technology Hamirpur  
Hamirpur (H.P.) — 177005, India
