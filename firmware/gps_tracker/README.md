# GPS Tracker Firmware

Modular ESP32 firmware for PPA-TAC. On Firebase **Spark**, the board signs in with a device Auth user and writes `location_records` through the **Firestore REST API**. Cloud Functions are not used.

Phone GPS in the parolee app remains a backup.

## Hardware Support

- **MCU:** ESP32
- **GPS:** NEO-6M, L86, or any NMEA module via TinyGPSPlus
- **Connectivity:** Wi-Fi (or GSM/GPRS if you replace `NetworkService`)
- **Battery:** Optional ADC via voltage divider

## Project Structure

```
firmware/gps_tracker/
├── gps_tracker.ino      # Main loop
├── device_config.h      # Credentials and intervals
├── gps_service.h        # GPS reading
├── battery_service.h    # Battery level
├── network_service.h    # Wi-Fi
└── api_client.h         # Auth + Firestore REST
```

## Officer + firmware setup

1. In the officer app, create a GPS tracker. Note **TRACKER_ID** (Firestore document id).
2. Assign the tracker to a parolee.
3. Firebase Console → Authentication → enable Email/Password → Add user, e.g. `trk-001@pptac-attendance-capstone.web.app`. Do **not** create a `users/{uid}` profile.
4. Copy the Auth UID. On the tracker device screen, paste it as **Device Auth UID** (`ingestAuthUid`).
5. Edit `device_config.h` (do not commit real passwords):

```cpp
#define TRACKER_ID "TRK-001"
#define DEVICE_EMAIL "trk-001@pptac-attendance-capstone.web.app"
#define DEVICE_PASSWORD "the-password-you-set-in-console"
#define FIREBASE_WEB_API_KEY "web apiKey from lib/firebase_options.dart"
#define FIREBASE_PROJECT_ID "pptac-attendance-capstone"
```

6. Set Wi-Fi SSID/password in `gps_tracker.ino`.
7. Flash the ESP32. After a GPS fix and sign-in, it:
   - GETs `gps_trackers/{TRACKER_ID}`
   - POSTs `location_records` with `source: "esp32"`
   - PATCHes `lastSeenAt`, `batteryLevel`, `status`, `updatedAt`

If Identity Toolkit fails, in Google Cloud restrict the **web** API key to Identity Toolkit + Firestore (no HTTP referrer), do not disable the key.

Do not deploy Cloud Functions on Spark. `functions/index.js` is leftover Blaze ingest only.

## Transmission Strategy

| State | Interval |
|-------|----------|
| Moving (>3 km/h) | 60 seconds |
| Normal | 90 seconds |
| Stationary | 120 seconds |

Slower pings stay near Spark’s ~20,000 Firestore writes/day.

## Features

- Firebase Auth sign-in with cached id token
- Writes only when the tracker is assigned
- Offline queue (up to 20 records)
- Duplicate coordinate suppression
- Modular services

## Required Libraries

Install via Arduino Library Manager:

- **TinyGPSPlus** — GPS NMEA parsing
- **ArduinoJson 6.x** — JSON serialization (`DynamicJsonDocument` / `createNestedObject`)

## GSM Module Notes

For SIM800L/SIM7600, replace `network_service.h` with an AT-command HTTPS client. Auth and Firestore REST URLs stay the same.

## Security

- Never commit device passwords
- Use HTTPS with a proper CA in production (remove `setInsecure()`)
- Rotate a compromised device by changing the Auth password and/or linking a new UID in the officer app
