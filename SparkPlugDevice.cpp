#include "SparkPlugDevice.h"

void SparkPlugDevice::setLogging(bool enabled) {
  this->logging = enabled;
  this->net.setLogging(enabled);
}

void SparkPlugDevice::setup(
  String ssid, String wifiPassword,
  String mqttHost, int mqttPort, String mqttUsername, String mqttPassword,
  String groupId, String nodeId, String deviceId
) {
  // Node topics
  this-> nodeBirthTopic = String(SPARKPLUG_B_NAMESPACE) + "/" + groupId + "/NBIRTH/" + nodeId;
  String nDeathTopic = String(SPARKPLUG_B_NAMESPACE) + "/" + groupId + "/NDEATH/" + nodeId;
  this->heartbeatTopic = String(SPARKPLUG_B_NAMESPACE) + "/" + groupId + "/HEARTBEAT/" + nodeId;

  // Device topics
  this->deviceBirthTopic = String(SPARKPLUG_B_NAMESPACE) + "/" + groupId + "/DBIRTH/" + nodeId + "/" + deviceId;
  this->deviceDataTopic = String(SPARKPLUG_B_NAMESPACE) + "/" + groupId + "/DDATA/" + nodeId + "/" + deviceId;
  this->deviceCommandTopic = String(SPARKPLUG_B_NAMESPACE) + "/" + groupId + "/DCMD/" + nodeId + "/" + deviceId;

  String nDeathMessage = "NDEATH";

  this->net.setupWifi(ssid, wifiPassword, nodeId);
  this->net.setConnectedCallback([this] () { this->connectedCallback(); });
  this->net.setDataCallback([this] (char* topic, byte* payload, unsigned int length) { this->dataCallback(topic, payload, length); });
  this->net.setupMqtt(mqttHost, mqttPort, nodeId, mqttUsername, mqttPassword, nDeathTopic, nDeathMessage);
}

void SparkPlugDevice::setConnectedCallback(std::function<void()> connectedCallback) {
  this->connectedCallbackFunction = connectedCallback;
}

void SparkPlugDevice::setDeviceCommandCallback(std::function<void(char*)> deviceCommandCallback) {
  this->deviceCommandCallback = deviceCommandCallback;
}

void SparkPlugDevice::loop() {
  this->net.loop();
}

// Not really Sparkplug_B Remove when stable!
void SparkPlugDevice::heartbeat() {
  if(this->logging) {
    Serial.println("=== heartbeat() ===");
  }

  this->net.publishMqtt(this->heartbeatTopic, "alive");
}

void SparkPlugDevice::publishDeviceBirth(DynamicJsonDocument doc) {
  String message = "";
  serializeJson(doc, message);

  this->net.publishMqtt(this->deviceBirthTopic, message.c_str());
}

void SparkPlugDevice::publishDeviceData(DynamicJsonDocument doc) {
  String message = "";
  serializeJson(doc, message);

  this->net.publishMqtt(this->deviceDataTopic, message.c_str());
}

void SparkPlugDevice::publishNodeBirth() {
  DynamicJsonDocument doc(1024);
  JsonArray metrics = doc.createNestedArray("metrics");

  JsonObject metricIp  = metrics.createNestedObject();
  metricIp["name"]="IP";
  metricIp["value"]=this->net.getWifi().localIP().toString();

  JsonObject metricRssi = metrics.createNestedObject();
  metricRssi["name"]="RSSI";
  metricRssi["value"]=this->net.getWifi().RSSI();

  String message = "";
  serializeJson(doc, message);
  this->net.publishMqtt(this->nodeBirthTopic, message);
}

void SparkPlugDevice::dataCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';

  if(this->logging) {
    String strPayload = String((char*)payload);
    Serial.println("=== MQTT Message Received! ===");
    Serial.print("\tTopic: \t");
    Serial.println(topic);
    Serial.println("\tMessage:");
    
    Serial.println(strPayload);
  }

  String strTopic = String(topic);
  if (strTopic.equals(this->deviceCommandTopic)) {
    if(this->logging) {
      Serial.println("Received DCMD - Device Command");
    }
    if(this->deviceCommandCallback) {
      deviceCommandCallback((char*)payload);
    }
  }
}

void SparkPlugDevice::connectedCallback() {
  if(this->logging) {
    Serial.println("Subscribing to Device Command");
    Serial.printf("  Topic:  %s\n", this->deviceCommandTopic.c_str());
  }
  this->net.subscribe(this->deviceCommandTopic);

  this->publishNodeBirth();
  this->connectedCallbackFunction();
}
