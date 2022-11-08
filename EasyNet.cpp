#include "EasyNet.h"

static const char* wifiStatusName[] = { "WL_IDLE_STATUS", "WL_NO_SSID_AVAIL", "WL_SCAN_COMPLETED", "WL_CONNECTED", "WL_CONNECT_FAILED", "WL_CONNECTION_LOST", "WL_WRONG_PASSWORD", "WL_DISCONNECTED" };

void EasyNet::setLogging(bool enabled) {
  this->logging = enabled;
}

void EasyNet::setupWifi(String ssid, String password, String hostname) {
  this->configureWifi(ssid, password, hostname);
  this->connectWifi();
}

void EasyNet::configureWifi(String ssid, String password, String hostname) {
  this->firstConnect = true;
  this->wifiWasConnectedLastLoop = false;
  this->ssid = ssid;
  this->wifiPassword = password;
  this->hostname = hostname;
  WiFi.persistent(false);
  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA);

  if(this->logging) {
    Serial.println("Configuring WiFi");
    Serial.printf("  MAC:         %s\n", WiFi.macAddress().c_str());
    Serial.printf("  SSID:        %s\n", this->ssid.c_str());
    Serial.printf("  Hostname:    %s\n", WiFi.hostname());
    Serial.printf("  AutoConnect: %s\n", WiFi.getAutoConnect() ? "true" : "false");
  }
}

void EasyNet::connectWifi() {
  if(this->logging) {
    Serial.println("Trying to connect to WiFi");
  }
  WiFi.begin(this->ssid, this->wifiPassword);
}

void EasyNet::loop() {
  this->loopWifi();
  
  if(this->mqttConfigured) {
    this->loopMqtt();
  }
}

void EasyNet::loopWifi() {
  wl_status_t wifiStatus = WiFi.status();
  if(!this->wifiWasConnectedLastLoop && wifiStatus == WL_CONNECTED) {
    this->wifiWasConnectedLastLoop = true;
    if(this->firstConnect) {
      this->firstConnect = false;
      this->wifiConnected(wifiStatus);
    } else {
      this->wifiReconnected(wifiStatus);
    }
  } else if(this->wifiWasConnectedLastLoop && wifiStatus != WL_CONNECTED) {
    this->wifiWasConnectedLastLoop = false;
    this->firstStatusMillis = millis();
      this->statusMillis = millis();
    this->wifiDisconnected(wifiStatus);
  } else if(!this->wifiWasConnectedLastLoop && wifiStatus != WL_CONNECTED) {
    if(millis() - this->statusMillis >= DISCONNECTED_STATUS_INTERVAL) {
      this->statusMillis = millis();
      this->wifiStillDisconnected(wifiStatus, this->statusMillis - this->firstStatusMillis);
    }
  }
}

void EasyNet::loopMqtt() {
  if(this->isWifiConnected() && this->mqttFirstConnect) {
    this->connectMqtt();
    this->mqttFirstConnect = false;
  } else if(!this->mqttFirstConnect) {
    if (this->isWifiConnected() && !mqtt.connected()) {
      this->reconnectMqtt();
    }
  }
  mqtt.loop();
}

ESP8266WiFiClass EasyNet::getWifi() {
  return WiFi;
}

wl_status_t EasyNet::getWifiStatus() {
  return WiFi.status();
}

String EasyNet::getWifiStatusString() {
  return wifiStatusName[this->getWifiStatus()];
}

bool EasyNet::isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void EasyNet::wifiConnected(wl_status_t wifiStatus) {
  if(this->logging) {
    Serial.println("WiFi Connected!");
    Serial.printf("  Status:     %s\n", wifiStatusName[wifiStatus]);
    Serial.printf("  IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  RRSI:       %d\n", WiFi.RSSI());
  }
}

void EasyNet::wifiReconnected(wl_status_t wifiStatus) {
  if(this->logging) {
    Serial.println("WiFi Reconnected!");
    Serial.printf("  Status:     %s\n", wifiStatusName[wifiStatus]);
    Serial.printf("  IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  RRSI:       %d\n", WiFi.RSSI());
  }
}

void EasyNet::wifiDisconnected(wl_status_t wifiStatus) {
  if(this->logging) {
    Serial.printf("WiFi Disconnected: %d\n", wifiStatus);
  }
}

void EasyNet::wifiStillDisconnected(wl_status_t wifiStatus, int millis) {
  if(this->logging) {
    Serial.printf("WiFi still disconnected after %d seconds. Status: %s\n", int(millis / 1000), wifiStatusName[wifiStatus]);
  }
}

void EasyNet::setupMqtt(String host, int port, String clientId, String username, String password, String lastWillTopic, String lastWillMessage) {
  this->mqttFirstConnect = true;
  this->mqttHost = host;
  this->mqttUsename = username;
  this->mqttClientId = clientId;
  this->mqttPort = port;
  this->mqttPassword = password;
  this->lastWillTopic = lastWillTopic;
  this->lastWillMessage = lastWillMessage;

  mqtt.setBufferSize(1024);
  mqtt.setClient(this->wifiClient);
  mqtt.setServer(this->mqttHost.c_str(), this->mqttPort);

  if(this->logging) {
    Serial.println("Configuring MQTT");
    Serial.printf("  Host:        %s\n", this->mqttHost.c_str());
    Serial.printf("  Port:        %d\n", this->mqttPort);
    Serial.printf("  Client ID:   %s\n", this->mqttClientId.c_str());
    Serial.printf("  Username:    %s\n", this->mqttUsename.c_str());
    Serial.printf("  Buffer Size: %d\n", this->mqtt.getBufferSize());
  }
  this->mqttConfigured = true;
}

void EasyNet::connectMqtt() {
  if(this->logging) {
    Serial.println("Trying to connect to MQTT");
  }

  bool connected = this->tryConnectMqtt();
  if(connected) {
    if(this->logging) {
      Serial.println("MQTT Connected!");
    }
    if(this->connectedCallback) {
      connectedCallback();
    }
  } else {
    if(this->logging) {
      Serial.printf("Failed to connect to MQTT, rc=%d\n", mqtt.state());
    }
  }
}

void EasyNet::reconnectMqtt() {
  if(this->logging) {
    Serial.println("Trying to reconnect to MQTT");
  }

  bool connected = this->tryConnectMqtt();
  if(connected) {
    if(this->logging) {
      Serial.println("MQTT reconnected!");
    }
    if(this->connectedCallback) {
      connectedCallback();
    }
  } else {
    if(this->logging) {
      Serial.printf("Failed to connect to MQTT, rc=%d\n", mqtt.state());
    }
  }
}

bool EasyNet::tryConnectMqtt() {
  return mqtt.connect(this->mqttClientId.c_str(), this->mqttUsename.c_str(), this->mqttPassword.c_str(), this->lastWillTopic.c_str(), 1, false, this->lastWillMessage.c_str());
}

void EasyNet::subscribe(String topic) {
  if(this->logging) {
    Serial.println("Subscribing to topics");
  }
  mqtt.subscribe(topic.c_str());
}

bool EasyNet::isMqttConnected() {
  return this->mqtt.connected();
}

bool EasyNet::publishMqtt(String topic, String payload) {
  if(this->logging) {
    Serial.printf("Publish to MQTT Topic: %s\n>>>\n%s\n<<<\n", topic.c_str(), payload.c_str());
  }
  if(!this->isWifiConnected()) {
    if(this->logging) {
      Serial.println("WiFi not connected, will not publish MQTT message.");
    }
    return false;
  }

  if(!this->isMqttConnected()) {
    if(this->logging) {
      Serial.println("MQTT not connected, will not publish MQTT message.");
    }
    return false;
  }
  // calculate new setBufferSize?
  bool succeess = mqtt.publish(topic.c_str(), payload.c_str());
  if(this->logging) {
    if(succeess) {
      Serial.println("Successfully published MQTT message");
    } else {
      Serial.println("Failed to publish MQTT message");
    }
  }
  return succeess;
}

int EasyNet::getMqttStatus() {
  return mqtt.state();
}

String EasyNet::mqttStatusToString(int status) {
  switch (status) {
    case -4:
      return "MQTT_CONNECTION_TIMEOUT";
    case -3:
      return "MQTT_CONNECTION_LOST";
    case -2:
      return "MQTT_CONNECT_FAILED";
    case -1:
      return "MQTT_DISCONNECTED";
    case 0:
      return "MQTT_CONNECTED";
    case 1:
      return "MQTT_CONNECT_BAD_PROTOCOL";
    case 2:
      return "MQTT_CONNECT_BAD_CLIENT_ID";
    case 3:
      return "MQTT_CONNECT_UNAVAILABLE";
    case 4:
      return "MQTT_CONNECT_BAD_CREDENTIALS";
    case 5:
      return "MQTT_CONNECT_UNAUTHORIZED";
    default:
      return "UNKNOWN";
  }
}

String EasyNet::getMqttStatusString() {
  return this->mqttStatusToString(this->getMqttStatus());
}

void EasyNet::setDataCallback(MQTT_CALLBACK_SIGNATURE) {
  this->mqtt.setCallback(callback);
}

void EasyNet::setConnectedCallback(std::function<void()> callback) {
  this->connectedCallback = callback;
}