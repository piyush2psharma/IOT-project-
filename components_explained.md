# Smart Door Control System — Component Guide
## B.Tech / M.Tech IoT Project

---

## Component Roles & Significance

---

### 1. ESP32 Dev Board — *The Brain*

The **microcontroller** that runs the entire program. It:
- Processes all sensor inputs
- Controls all outputs (servo, LEDs, buzzer, LCD)
- Connects to WiFi and communicates with ThingSpeak cloud

**Why ESP32?**
Because it has **built-in WiFi + Bluetooth**, making it perfect for IoT
projects. A regular Arduino cannot do cloud communication natively.

---

### 2. RFID RC522 + Cards — *The Authentication System*

The **Radio Frequency Identification** reader scans RFID cards/tags. It:
- Reads the unique **UID (Unique ID)** from each card
- Compares it against a list of authorized UIDs stored in the code
- If matched → grants access; if not → denies access

**Significance:**
This is the core of the "smart" door — replaces traditional keys with
digital card-based authentication. Used in real-world office buildings,
hotels, and metro systems.

---

### 3. Servo Motor SG90 — *The Physical Lock*

Acts as the **door lock mechanism**. It:
- Rotates to **0°**  = Door Locked
- Rotates to **90°** = Door Unlocked
- Auto-locks after **5 seconds** automatically

**Significance:**
Provides the actual physical action of locking/unlocking, simulating a
real electronic door latch or deadbolt.

---

### 4. I2C LCD 16×2 — *The Display Interface*

Shows **real-time system status** to the user. Displays:
- `SMART DOOR SYS`  — on boot
- `WiFi Connected / Offline`
- `ACCESS GRANTED` / `ACCESS DENIED`
- `MOTION DETECTED`
- `DOOR: OPEN` / `DOOR: LOCKED` + Cloud status

**Significance:**
Makes the system user-friendly. Without it, the user has no local
feedback about what is happening.

---

### 5. PIR Sensor (HC-SR501) — *Motion Detection*

**Passive Infrared Sensor** detects human movement near the door. It:
- Detects body heat (infrared radiation) when someone approaches
- Triggers an alert on the LCD and flashes the red LED
- Prompts the user to scan their card
- Sends a motion alert to ThingSpeak cloud

**Significance:**
Adds a **security layer** — the system knows someone is near the door
even before a card is scanned. Useful for logging suspicious activity.

---

### 6. Three LEDs — *Visual Feedback*

| LED   | Color  | Meaning                            |
|-------|--------|------------------------------------|
| Green | 🟢     | Access **Granted**                 |
| Red   | 🔴     | Access **Denied** / Motion Alert   |
| Blue  | 🔵     | WiFi connecting / Cloud uploading  |

**Significance:**
Instant visual indication so even someone standing far away can see
the door status at a glance.

---

### 7. Buzzer — *Audio Feedback*

Gives sound alerts based on the event:
- **2 short beeps** = Access Granted ✅
- **3 long beeps**  = Access Denied  ❌

**Significance:**
Provides audio confirmation so the user does not need to look at the
LCD. Essential for accessibility and real-world usability.

---

### 8. ThingSpeak Cloud — *Remote Monitoring*

The **IoT cloud platform** that receives and visualizes data. It:
- Logs door status, access results, motion alerts, and attempt count
- Plots **real-time graphs** viewable from anywhere in the world
- Data is sent every **15 seconds** + immediately on every card scan

**Channel Fields:**

| Field   | Name             | Values                        |
|---------|------------------|-------------------------------|
| Field 1 | Door Status      | 1 = Open,    0 = Closed       |
| Field 2 | Auth Result      | 1 = Granted, 0 = Denied       |
| Field 3 | Motion Alert     | 1 = Detected, 0 = Clear       |
| Field 4 | Total Attempts   | Running count of card scans   |

**Significance:**
This is what elevates it from a local embedded project to a full
**IoT project**. You can remotely monitor who is accessing the door,
when, and how many attempts were made — from your phone or laptop.

---

## System Architecture Summary

```
 RFID Card Scan
      │
      ▼
 ESP32 checks UID
      │
      ├── Authorised? ──Yes──► Open Servo (90°)
      │                        Green LED ON
      │                        2 Beeps
      │                        LCD: ACCESS GRANTED
      │
      └── Not Authorised? ──► Stay Locked (0°)
                               Red LED ON
                               3 Beeps
                               LCD: ACCESS DENIED
                                    │
                                    ▼
                              ThingSpeak Upload
                         (Field1, Field2, Field3, Field4)
                                    │
                                    ▼
                           Real-time Cloud Graph
```

---

## Why This is a B.Tech / M.Tech Level Project

Every component has a **distinct, real-world purpose** — no component
is redundant. The combination of:

- **Hardware Authentication** (RFID)
- **Physical Actuation** (Servo Motor)
- **Environmental Sensing** (PIR)
- **Multi-modal Feedback** (LCD + LEDs + Buzzer)
- **Cloud IoT Integration** (ThingSpeak)
- **WiFi-enabled Microcontroller** (ESP32)

...makes this a **complete, end-to-end IoT Security System** that
demonstrates skills in embedded systems, sensor interfacing, networking,
and cloud data visualization — all core competencies of a B.Tech / M.Tech
graduate in Electronics / Computer Engineering.

---

*Smart IoT Lab — 2026*
