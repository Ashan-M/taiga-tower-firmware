#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

bool performOTA(const String &firmwareURL)
{
    Serial.println();
    Serial.println("================================");
    Serial.println("        OTA UPDATE START        ");
    Serial.println("================================");

    WiFiClient client;
    HTTPClient http;

    Serial.println("[OTA] URL:");
    Serial.println(firmwareURL);

    if (!http.begin(client, firmwareURL)) {
        Serial.println("[OTA] HTTP begin failed");
        return false;
    }

    int httpCode = http.GET();

    Serial.printf("[OTA] HTTP response: %d\n", httpCode);

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[OTA] Download failed: %d\n", httpCode);
        http.end();
        return false;
    }

    int contentLength = http.getSize();

    Serial.printf("[OTA] Firmware size: %d bytes\n", contentLength);

    if (contentLength <= 0) {
        Serial.println("[OTA] Invalid firmware size");
        http.end();
        return false;
    }

    if (!Update.begin(contentLength)) {
        Serial.printf(
            "[OTA] Update.begin failed: %s\n",
            Update.errorString()
        );

        http.end();
        return false;
    }

    Serial.println("[OTA] Writing firmware...");

    WiFiClient *stream = http.getStreamPtr();

    size_t written = Update.writeStream(*stream);

    Serial.printf(
        "[OTA] Written: %u / %u bytes\n",
        written,
        contentLength
    );

    if (written != contentLength) {
        Serial.println("[OTA] Write incomplete");

        Update.abort();
        http.end();

        return false;
    }

    if (!Update.end()) {
        Serial.printf(
            "[OTA] Update.end failed: %s\n",
            Update.errorString()
        );

        http.end();
        return false;
    }

    if (!Update.isFinished()) {
        Serial.println("[OTA] Update is not finished");

        http.end();
        return false;
    }

    Serial.println("[OTA] Update successful!");

    http.end();

    Serial.println("[OTA] Rebooting...");

    delay(1000);

    ESP.restart();

    return true;
}

// bool performOTA(const String &firmwareURL)
// {
//     Serial.println();
//     Serial.println("================================");
//     Serial.println("        OTA UPDATE START        ");
//     Serial.println("================================");

//     WiFiClientSecure client;

//     // Temporary for testing.
//     // We will replace this with proper certificate validation later.
//     client.setInsecure();

//     HTTPClient http;

//     Serial.println("[OTA] Firmware URL:");
//     Serial.println(firmwareURL);

//     if (!http.begin(client, firmwareURL)) {
//         Serial.println("[OTA] HTTP begin failed");
//         return false;
//     }

//     int httpCode = http.GET();

//     Serial.printf("[OTA] HTTP response: %d\n", httpCode);

//     if (httpCode != HTTP_CODE_OK) {
//         Serial.println("[OTA] Firmware download failed");
//         http.end();
//         return false;
//     }

//     int contentLength = http.getSize();

//     Serial.printf("[OTA] Firmware size: %d bytes\n", contentLength);
//     Serial.printf("[OTA] Free OTA space: %u bytes\n",
//                   ESP.getFreeSketchSpace());

//     if (contentLength <= 0) {
//         Serial.println("[OTA] Invalid firmware size");
//         http.end();
//         return false;
//     }

//     if (contentLength > ESP.getFreeSketchSpace()) {
//         Serial.println("[OTA] Firmware too large");
//         http.end();
//         return false;
//     }

//     if (!Update.begin(contentLength)) {
//         Serial.printf("[OTA] Update.begin failed. Error: %s\n",
//                       Update.errorString());
//         http.end();
//         return false;
//     }

//     Serial.println("[OTA] Writing firmware...");

//     WiFiClient *stream = http.getStreamPtr();

//     size_t written = Update.writeStream(*stream);

//     Serial.printf("[OTA] Written: %u / %u bytes\n",
//                   written,
//                   contentLength);

//     if (written != contentLength) {
//         Serial.println("[OTA] Firmware write incomplete");

//         Update.abort();
//         http.end();

//         return false;
//     }

//     if (!Update.end()) {
//         Serial.printf("[OTA] Update.end failed. Error: %s\n",
//                       Update.errorString());

//         http.end();
//         return false;
//     }

//     if (!Update.isFinished()) {
//         Serial.println("[OTA] Update not finished");

//         http.end();
//         return false;
//     }

//     Serial.println("[OTA] Firmware successfully written!");

//     http.end();

//     Serial.println("[OTA] Rebooting ESP32...");

//     delay(1000);

//     ESP.restart();

//     return true;
// }