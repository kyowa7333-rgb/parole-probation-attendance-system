/**
 * GPS Tracker Firmware — PPA-TAC (Spark Firestore REST)
 *
 * TRACKER_ID must match the Firestore gps_trackers document id.
 * DEVICE_EMAIL / DEVICE_PASSWORD are the Firebase Auth user created in the console.
 * FIREBASE_WEB_API_KEY is DefaultFirebaseOptions.web.apiKey in lib/firebase_options.dart.
 * Never commit real device passwords.
 */

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#define TRACKER_ID "TRK-001"
#define DEVICE_ID TRACKER_ID
#define DEVICE_EMAIL "trk-001@pptac-attendance-capstone.web.app"
#define DEVICE_PASSWORD "your-device-password-here"

#define FIREBASE_WEB_API_KEY "AIzaSyBUfgqd43tnEt0KOl-nztE5KgzV0k_8Sag"
#define FIREBASE_PROJECT_ID "pptac-attendance-capstone"

// Dev only: skip TLS cert check. Set 0 and load a Google Trust Services CA for production.
#define USE_INSECURE_TLS 1

// Spark-safe intervals (milliseconds)
#define INTERVAL_MOVING_MS 60000
#define INTERVAL_NORMAL_MS 90000
#define INTERVAL_STATIONARY_MS 120000

#define MOVING_SPEED_KMH 3.0

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GSM_RX_PIN 18
#define GSM_TX_PIN 19
#define BATTERY_ADC_PIN 34

#define MAX_QUEUE_SIZE 20

#endif
