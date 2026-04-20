# -*- coding: utf-8 -*-
import sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
"""
=============================================================
  SMART DOOR CONTROL SYSTEM — ThingSpeak Data Populator
=============================================================
  This script sends realistic simulated data to ThingSpeak
  so your channel graphs look full and meaningful.

  Fields:
    Field 1 → Door Status      (1=Open, 0=Closed)
    Field 2 → Auth Result      (1=Granted, 0=Denied)
    Field 3 → Motion Alert     (1=Detected, 0=Clear)
    Field 4 → Total Attempts   (running count)

  Run:  python populate_thingspeak.py
=============================================================
"""

import requests
import time
import random

# ── ThingSpeak Config ──────────────────────────────────────
API_KEY    = "GJTXKOL4MA0C0V7T"   # Your Write API Key
TS_URL     = "https://api.thingspeak.com/update"
DELAY      = 16   # seconds between sends (ThingSpeak min = 15s)

# ── Realistic Event Sequences ──────────────────────────────
# Each tuple: (door_status, auth_result, motion_alert, total_attempts)
events = [
    # Morning arrivals
    (0, 0, 0, 0),    # System idle, no motion
    (0, 0, 1, 0),    # Motion detected, no card yet
    (1, 1, 1, 1),    # Card scanned → Granted → Door opens
    (0, 1, 0, 1),    # Door auto-locks
    (0, 0, 0, 1),    # Idle
    (0, 0, 1, 1),    # Motion detected
    (1, 1, 1, 2),    # Granted → Door opens
    (0, 1, 0, 2),    # Auto-lock
    (0, 0, 1, 2),    # Motion near door
    (0, 0, 2),       # Unknown card attempt
    (0, 0, 1, 3),    # Motion still there
    (0, 0, 1, 3),    # Waiting

    # Mid-morning — unauthorized attempt
    (0, 0, 1, 3),    # Motion detected
    (0, 0, 1, 4),    # Unknown card → Denied
    (0, 0, 1, 4),    # Motion still
    (0, 0, 0, 4),    # Motion cleared

    # Authorized entries
    (1, 1, 1, 5),    # Granted
    (0, 1, 0, 5),    # Auto-lock
    (0, 0, 0, 5),    # Idle
    (0, 0, 1, 5),    # Motion
    (1, 1, 1, 6),    # Granted
    (0, 1, 0, 6),    # Locked
    (0, 0, 0, 6),    # Idle
    (0, 0, 1, 6),    # Motion
    (0, 0, 6),       # Denied
    (0, 0, 0, 7),    # Cleared

    # Afternoon
    (0, 0, 0, 7),
    (0, 0, 1, 7),
    (1, 1, 1, 8),
    (0, 1, 0, 8),
    (0, 0, 0, 8),
    (0, 0, 1, 8),
    (0, 0, 9),
    (0, 0, 1, 9),
    (0, 0, 0, 9),
    (1, 1, 0, 10),
    (0, 1, 0, 10),

    # Evening — busy period
    (0, 0, 1, 10),
    (1, 1, 1, 11),
    (0, 1, 0, 11),
    (0, 0, 1, 11),
    (0, 0, 12),
    (0, 0, 1, 12),
    (1, 1, 1, 13),
    (0, 1, 0, 13),
    (0, 0, 0, 13),
    (0, 0, 1, 13),
    (0, 0, 14),
    (0, 0, 0, 14),
    (1, 1, 0, 15),
    (0, 1, 0, 15),
    (0, 0, 0, 15),
]

# Fix tuples that have only 3 values (missed total_attempts param)
def fix_event(e):
    if len(e) == 3:
        door, auth, motion = e
        return (door, auth, motion, 0)
    return e

events = [fix_event(e) for e in events]

# ── Send Function ──────────────────────────────────────────
def send_to_thingspeak(field1, field2, field3, field4):
    params = {
        "api_key": API_KEY,
        "field1":  field1,
        "field2":  field2,
        "field3":  field3,
        "field4":  field4,
    }
    try:
        response = requests.get(TS_URL, params=params, timeout=10)
        if response.status_code == 200 and response.text != "0":
            print(f"  [OK] Entry #{response.text} sent | "
                  f"Door={field1} Auth={field2} Motion={field3} Attempts={field4}")
        else:
            print(f"  [FAIL] status={response.status_code}, body={response.text}")
    except Exception as ex:
        print(f"  [ERR] {ex}")

# ── Main ───────────────────────────────────────────────────
def main():
    print("=" * 58)
    print("  SMART DOOR — ThingSpeak Data Populator")
    print("=" * 58)
    print(f"  API Key : {API_KEY}")
    print(f"  URL     : {TS_URL}")
    print(f"  Events  : {len(events)} data points to send")
    print(f"  Delay   : {DELAY}s between sends")
    print(f"  Total   : ~{len(events) * DELAY // 60} minutes to complete")
    print("=" * 58)
    print("  Starting in 3 seconds... (Press Ctrl+C to stop)\n")
    time.sleep(3)

    for i, (door, auth, motion, attempts) in enumerate(events, 1):
        print(f"[{i:02d}/{len(events)}] Sending event...", end=" ", flush=True)
        send_to_thingspeak(door, auth, motion, attempts)

        if i < len(events):
            print(f"         ⏳ Waiting {DELAY}s...")
            time.sleep(DELAY)

    print("\n" + "=" * 58)
    print("  [DONE] All data sent! Check your ThingSpeak channel:")
    print("  https://thingspeak.com/channels")
    print("=" * 58)

if __name__ == "__main__":
    main()
