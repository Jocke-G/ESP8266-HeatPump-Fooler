#ifndef EASY_NET_H
#define EASY_NET_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <functional>

#define DISCONNECTED_STATUS_INTERVAL 1000

class EasyNet {
  private:
    bool logging;

    WiFiClient wifiClient;
    String ssid;
    String wifiPassword;
    String hostname;

    bool firstConnect;
    bool wifiWasConnectedLastLoop;

    int firstStatusMillis;
    int statusMillis;

    // MQTT
    PubSubClient mqtt;
    String mqttHost;
    int mqttPort;
    String mqttClientId;
    String mqttUsename;
    String mqttPassword;
    String lastWillTopic;
    String lastWillMessage;

    bool mqttConfigured;
    bool mqttFirstConnect;

    void wifiConnected(wl_status_t wifiStatus);
    void wifiReconnected(wl_status_t wifiStatus);
    void wifiDisconnected(wl_status_t wifiStatus);
    void wifiStillDisconnected(wl_status_t wifiStatus, int millis);

    std::function<void()> connectedCallback;

  public:
    // Common - Both WiFi and MQTT
    void setLogging(bool enabled);
    void loop();

    // WiFi
    /**
     * Configure and Connect WiFi
     * @param ssid
     * @param password
     * @param hostname
     */
    void setupWifi(String ssid, String password, String hostname);
    /**
     * Configure WiFi
     * @param ssid
     * @param password
     * @param hostname
     */
    void configureWifi(String ssid, String password, String hostname);
    /**
     * Connect WiFi
     */
    void connectWifi();
    /**
     * Loop WiFi. Call in each loop, to maintain connection.
     */
    void loopWifi();

    ESP8266WiFiClass getWifi();
    bool isWifiConnected();
    wl_status_t getWifiStatus();
    String getWifiStatusString();

    // MQTT
    void setupMqtt(String host, int port, String clientId, String username, String password, String lastWillTopic, String lastWillMessage);
    void connectMqtt();
    void reconnectMqtt();
    bool tryConnectMqtt();
    void loopMqtt();
    bool publishMqtt(String topic, String payload);
    bool isMqttConnected();
    int getMqttStatus();
    String mqttStatusToString(int status);
    String getMqttStatusString();

    void subscribe(String topic);
    void setConnectedCallback(std::function<void()> callback);
    void setDataCallback(MQTT_CALLBACK_SIGNATURE);
};
#endif
