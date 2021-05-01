#pragma once
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266WiFi.h>




struct MQTT {
    typedef std::function<void(char*)> callback_function_t;
    
    MQTT(callback_function_t on_command);
    void setup();
    void run();
    
    template <typename T>
    bool publish_height(T x) {
        return height_.publish(x);
    }
    
    WiFiClientSecure client_;
    Adafruit_MQTT_Client mqtt_;
    Adafruit_MQTT_Publish height_;
    Adafruit_MQTT_Subscribe commands_;
    callback_function_t on_command_;
};