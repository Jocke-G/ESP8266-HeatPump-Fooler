#ifndef SPARKPLUG_DEVICE_H
#define SPARKPLUG_DEVICE_H

#include <Arduino.h>
#include "SparkPlug_B.h"
#include "EasyNet.h"

class SparkPlugDevice {
  #define SPARKPLUG_B_NAMESPACE "spBv1.0"

  private:
    bool logging;
    EasyNet net;
    // Node topics
    String nodeBirthTopic;
    String heartbeatTopic;

    // Device topics
    String deviceBirthTopic;
    String deviceDataTopic;
    String deviceCommandTopic;
    std::function<void()> connectedCallbackFunction;
    std::function<void(char*)> deviceCommandCallback;

  public:
    void setLogging(bool enabled);
    void setup(
        String ssid, String wifiPassword,
        String mqttHost, int mqttPort, String mqttUsername, String mqttPassword,
      	String groupId, String nodeId, String deviceId
    );
    void setConnectedCallback(std::function<void()> connectedCallback);
    void setDeviceCommandCallback(std::function<void(char*)> deviceCommandCallback);
    void loop();
    void heartbeat();

    void connectedCallback();
    void dataCallback(char* topic, byte* payload, unsigned int length);

    void publishNodeBirth();
    void publishDeviceBirth(DynamicJsonDocument doc);

    void publishDeviceData(DynamicJsonDocument doc);
};
#endif
