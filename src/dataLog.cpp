#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "pods.h"

const char* API_URL = "https://taiga-tower-backend.vercel.app/devices/001002/pods/data-log";

bool sendPodData()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[HTTP] WiFi not connected");
        return false;
    }

    HTTPClient http;

    http.begin(API_URL);
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;

    doc["deviceID"] = "001002";

    JsonArray podArray = doc["pods"].to<JsonArray>();

    for (int i = 0; i < NUM_PODS; i++)
    {
        JsonObject pod = podArray.add<JsonObject>();

        pod["podID"] = pods[i].podID;
        pod["moistureLevel"] = pods[i].currentMoisture;
        pod["lightIntensity"] = pods[i].targetLight;
        // pod["moistureLevel"] = random(0,100);
        // pod["lightIntensity"] = random(0,100);
    }

    String payload;

    serializeJson(doc, payload);

    Serial.println("[HTTP] Sending pod data:");
    Serial.println(payload);

    int httpCode = http.POST(payload);

    if (httpCode > 0)
    {
        Serial.print("[HTTP] Response code: ");
        Serial.println(httpCode);

        String response = http.getString();

        Serial.println("[HTTP] Response:");
        Serial.println(response);

        http.end();

        return httpCode >= 200 && httpCode < 300;
    }

    Serial.print("[HTTP] Request failed: ");
    Serial.println(http.errorToString(httpCode));

    http.end();

    return false;
}