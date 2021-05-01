#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "secrets.h"
#include "IO.h"

IO io;

size_t LOG(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return 0;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }

    len = Serial.write((const uint8_t*) buffer, len);  
    // io.telnet_.print(buffer);
    return len;
}


bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}


void IO::setup(bool useWifi) {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(SERIAL_SPEED);
    while (!Serial) {
    }
    delay(200);  

    if (!useWifi) {
        return;
    }

    WiFi.disconnect(); // remove old credentials they say
    WiFi.mode(WIFI_STA);
    WiFi.hostname("ESP8266");
    
    #if defined(ARDUINO_ARCH_ESP8266)
      WiFi.forceSleepWake();
      delay(200);
    #endif

    LOG("Connecting to wifi...");    
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    int count = 0;
    do {
      delay(500);
      Serial.printf("%d", WiFi.status());
    } while ((!isConnected()) && (count++ < 120));
    
    if (!isConnected()) {
        Serial.print("\nCould not connect to wifi. Resetting...");
        ESP.reset();
        delay(100);
        // unreachable;
    }
    
    delay(200);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    LOG("\nWiFi connted\nIP address: %s", WiFi.localIP().toString().c_str());
    delay(200);


	ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname("jarvis");
    ArduinoOTA.onStart([]() {
        LOG("OTA Start");
    });
    ArduinoOTA.onEnd([]() {
        LOG("OTA OnEnd!");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        LOG("OTA Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        digitalWrite(LED_BUILTIN, HIGH);
        LOG("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    // telnet_.begin();
}

void IO::run() {
    ArduinoOTA.handle();
    // telnet_.loop();
}