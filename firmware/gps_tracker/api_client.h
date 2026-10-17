#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "device_config.h"

struct LocationPayload {
  double latitude;
  double longitude;
  double accuracy;
  double speed;
  int batteryLevel;
  String timestamp;
};

class ApiClient {
public:
  bool sendLocation(const LocationPayload& loc) {
    if (!_ensureToken()) return false;

    String paroleeId;
    if (!_readAssignedParolee(paroleeId)) {
      return true;
    }

    if (!_postLocation(loc, paroleeId)) return false;
    _patchTrackerHeartbeat(loc);
    return true;
  }

private:
  String _idToken;
  unsigned long _tokenExpiresAtMs = 0;

  void _applyTls(WiFiClientSecure& client) {
#if USE_INSECURE_TLS
    client.setInsecure();
#else
    client.setInsecure();
#endif
  }

  String _firestoreDoc(const String& collection, const String& docId) {
    return String("https://firestore.googleapis.com/v1/projects/") +
           FIREBASE_PROJECT_ID +
           "/databases/(default)/documents/" + collection + "/" + docId;
  }

  bool _ensureToken() {
    if (_idToken.length() > 0 && millis() < _tokenExpiresAtMs) {
      return true;
    }

    WiFiClientSecure client;
    _applyTls(client);
    HTTPClient http;
    String url = String("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=") +
                 FIREBASE_WEB_API_KEY;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> req;
    req["email"] = DEVICE_EMAIL;
    req["password"] = DEVICE_PASSWORD;
    req["returnSecureToken"] = true;
    String body;
    serializeJson(req, body);

    int code = http.POST(body);
    String resp = http.getString();
    http.end();
    if (code != 200) {
      return false;
    }

    StaticJsonDocument<768> doc;
    if (deserializeJson(doc, resp)) return false;
    const char* token = doc["idToken"];
    if (token == nullptr || token[0] == '\0') return false;
    _idToken = String(token);
    long expiresIn = atol(doc["expiresIn"] | "3600");
    unsigned long skewMs = 5UL * 60UL * 1000UL;
    unsigned long ttlMs = expiresIn > 300 ? (unsigned long)(expiresIn * 1000L) - skewMs : 60000UL;
    _tokenExpiresAtMs = millis() + ttlMs;
    return true;
  }

  String _fieldString(JsonObject fields, const char* key) {
    if (!fields.containsKey(key)) return "";
    JsonObject fv = fields[key];
    if (fv.containsKey("nullValue")) return "";
    const char* v = fv["stringValue"];
    return v == nullptr ? "" : String(v);
  }

  bool _readAssignedParolee(String& paroleeId) {
    WiFiClientSecure client;
    _applyTls(client);
    HTTPClient http;
    http.begin(client, _firestoreDoc("gps_trackers", TRACKER_ID));
    http.addHeader("Authorization", "Bearer " + _idToken);

    int code = http.GET();
    String resp = http.getString();
    http.end();
    if (code != 200) return false;

    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, resp)) return false;
    JsonObject fields = doc["fields"];
    if (fields.isNull()) return false;
    paroleeId = _fieldString(fields, "paroleeId");
    return paroleeId.length() > 0;
  }

  bool _postLocation(const LocationPayload& loc, const String& paroleeId) {
    String docId = String(TRACKER_ID) + "-" + String(millis());
    String url = String("https://firestore.googleapis.com/v1/projects/") +
                 FIREBASE_PROJECT_ID +
                 "/databases/(default)/documents/location_records?documentId=" + docId;

    String recorded = loc.timestamp.length() > 0 ? loc.timestamp : "";

    DynamicJsonDocument doc(1536);
    JsonObject fields = doc.createNestedObject("fields");
    fields["id"]["stringValue"] = docId;
    fields["trackerId"]["stringValue"] = TRACKER_ID;
    fields["paroleeId"]["stringValue"] = paroleeId;
    fields["latitude"]["doubleValue"] = loc.latitude;
    fields["longitude"]["doubleValue"] = loc.longitude;
    fields["accuracy"]["doubleValue"] = loc.accuracy;
    fields["speed"]["doubleValue"] = loc.speed;
    fields["batteryLevel"]["integerValue"] = String(loc.batteryLevel);
    if (recorded.length() > 0) {
      fields["recordedAt"]["stringValue"] = recorded;
      fields["receivedAt"]["stringValue"] = recorded;
      fields["createdAt"]["stringValue"] = recorded;
    }
    fields["source"]["stringValue"] = "esp32";

    String body;
    serializeJson(doc, body);

    WiFiClientSecure client;
    _applyTls(client);
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + _idToken);
    int code = http.POST(body);
    http.end();
    return code == 200;
  }

  bool _patchTrackerHeartbeat(const LocationPayload& loc) {
    String recorded = loc.timestamp.length() > 0 ? loc.timestamp : "";
    String url = _firestoreDoc("gps_trackers", TRACKER_ID) +
                 "?updateMask.fieldPaths=lastSeenAt" +
                 "&updateMask.fieldPaths=batteryLevel" +
                 "&updateMask.fieldPaths=status" +
                 "&updateMask.fieldPaths=updatedAt";

    DynamicJsonDocument doc(768);
    JsonObject fields = doc.createNestedObject("fields");
    if (recorded.length() > 0) {
      fields["lastSeenAt"]["stringValue"] = recorded;
      fields["updatedAt"]["stringValue"] = recorded;
    }
    fields["batteryLevel"]["integerValue"] = String(loc.batteryLevel);
    fields["status"]["stringValue"] = "active";

    String body;
    serializeJson(doc, body);

    WiFiClientSecure client;
    _applyTls(client);
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + _idToken);
    int code = http.sendRequest("PATCH", body);
    http.end();
    return code == 200;
  }
};

#endif
