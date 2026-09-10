#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <nvs_flash.h>
#include <Preferences.h>
#include "globals.h"
#include "main.h"

String ssid = "";
String password = "";

WiFiState wifiState = WIFI_IDLE;

unsigned long lastBlink = 0;
bool ledState = false;


WebServer server(80);

// ------------ NVS HANDLE ---------------------------
void saveToNVS(String ssid, String password) {
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.end();
}

bool returnFromNVS() {
    Preferences prefs;
    prefs.begin("wifi", false);
    ssid = prefs.getString("ssid", ssid);
    password = prefs.getString("pass", password);
    prefs.end();

    if (ssid.length() > 0 && password.length() > 0) {
        Serial.println("SSID: " + ssid + "  Password: " + password);
        return true;
    } else{
        return false;
    }

}

void clearNVS() {
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.clear();   // 🔥 wipes all keys in "wifi"
    prefs.end();

    Serial.println("NVS Cleared!");
}


// -------------------- HANDLE INPUT --------------------

void handleSetWiFi() {
  String newSSSID = server.arg("ssid");
  String newPASS = server.arg("password");

  Serial.println("Received WiFi credentials:");
  Serial.println("SSID: " + newSSSID);
  Serial.println("PASS: " + newPASS);

  server.send(200, "text/plain", "Received. Restarting...");
  saveToNVS(newSSSID, newPASS);
  delay(1000);
  ESP.restart();
}

// -------------------- SERVER --------------------
void setupServer() {
  server.on("/", []() {
    server.send(200, "text/plain",
      "ESP32 AP Mode\nUse /setwifi?ssid=YOUR_SSID&password=YOUR_PASS");
  });

  server.on("/setwifi", HTTP_GET, handleSetWiFi);

  server.begin();
}

// -------------------- START AP --------------------
void startAP() {
  Serial.println("Starting AP...");
  WiFi.softAP("TaigaTower");

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  setupServer();
  wifiState = WIFI_AP_MODE;

//   while (true)
//   {
//     server.handleClient();
//   }
  
}


void connectToWifi() {

    if (!returnFromNVS()) {
        Serial.println("No saved WiFi. Starting AP...");
        startAP();
        return;
    }

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid.c_str(), password.c_str());

    wifiState = WIFI_CONNECTING;

    unsigned long startAttempt = millis();
    const unsigned long timeout = 15000; // 15 seconds

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttempt < timeout) {

        updateLED();  // 🔥 keep LED alive
        delay(50);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.println(WiFi.localIP());
        wifiState = WIFI_CONNECTED;
        wifi = true;
    } else {
        Serial.println("\nFailed! Starting AP...");
        wifi = false;
        startAP();
    }
}

void handleClient() {
    server.handleClient();
}