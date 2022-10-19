#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class EasyNet {
    private:
        String ssid;
        String password;
        String hostname;

    public:
        WiFiClient wifiClient;
        void configureWiFi(String ssid, String password, String hostname);
};
#endif
