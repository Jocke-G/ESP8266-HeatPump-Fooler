#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#if ONEWIRE_ENABLED
  #include <OneWire.h>
  #include <DallasTemperature.h>
#endif

#include "defines.h"
#include "MCP41X1.h"
#include "SparkPlug_B.h"
#include "thermistor.h"

MCP41X1 digiPot = MCP41X1(CS_PIN, MAX_OHMS, WIPER_RESISTANCE, POT_STEPS);

#if ONEWIRE_ENABLED
  OneWire oneWire(ONEWIRE_PIN);
  DallasTemperature sensors(&oneWire);
#endif

void mqttDataCallback(char* topic, byte* payload, unsigned int length);

int lastWorkMillis = 0;
int lastHeartbeatMillis = 0;
bool workNow = false;
boolean pendingDisconnect = true;

SteinhartHart steinhart = SteinhartHart(A, B, C);

WiFiClient wifi;
PubSubClient mqtt;

MODE currentMode = BYPASS; // BYPASS OFFSET FIXED
int offsetTemp = -2;
int fixedTemp = 25;
int workInterval = WORK_INTERVAL;

void mqttDataCallback(char* topic, byte* payload, unsigned int length) {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("=== MQTT Message Received! ===");
    Serial.print("\tTopic: \t");
    Serial.println(topic);
    Serial.print("\tMessage: \t");
    
    payload[length] = '\0';
    String s = String((char*)payload);
    Serial.println(s);
  }

  String strTopic = String(topic);
  if (strTopic.equals(MQTT_STATUS_REQUEST_TOPIC)) {
    String response = String("IP:");
    response += WiFi.localIP().toString();
    if(DEBUG_PRINT_SERIAL) {
      Serial.println("Publish Technical Status Report");
      Serial.print("\tTopic: \t");
      Serial.println(MQTT_STATUS_RESPONSE_TOPIC);
      Serial.print("\tMessage: \t");
      Serial.println(response);
    }
    mqtt.publish(MQTT_STATUS_RESPONSE_TOPIC, response.c_str());
  }

  if (strTopic.equals(MQTT_DCMD_TOPIC)) {
    if(DEBUG_PRINT_SERIAL) {
      Serial.println("Received DCMD - Device Command");
    }
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.f_str());
    }
    JsonArray arr = doc["metrics"];
    for (JsonObject repo : arr) {
      const char* name = repo["name"];
      Serial.print("Metric name:" );
      Serial.println(name);
      if(strcmp(name, "Mode") == 0) {
        if(strcmp(repo["value"], "FIXED") == 0) {
            currentMode = FIXED;
        }
        else if(strcmp(repo["value"], "OFFSET") == 0)
        {
          currentMode = OFFSET;
        }
        else if(strcmp(repo["value"], "BYPASS") == 0)
        {
            currentMode = BYPASS;
        }
        Serial.print("New Mode:" );
        Serial.println(currentMode);
      }
      if(strcmp(name, "OffsetTemp") == 0) {
        offsetTemp = String(repo["value"]).toInt();
        Serial.print("New OffsetTemp:" );
        Serial.println(offsetTemp);
      }
      if(strcmp(name, "FixedTemp") == 0) {
        fixedTemp = String(repo["value"]).toInt();
        Serial.print("New FixedTemp:" );
        Serial.println(fixedTemp);
      }
      if(strcmp(name, "WorkInterval") == 0) {
        workInterval = String(repo["value"]).toInt();
        Serial.print("New WorkInterval:" );
        Serial.println(workInterval);
      }
    }
    workNow = true;
  }
}

void mqttConnectedCallback() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("MQTT Connected");
  }

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Subscribing to Device Command");
    Serial.print("\tTopic:\t");
    Serial.println(MQTT_DCMD_TOPIC);
  }
  mqtt.subscribe(MQTT_DCMD_TOPIC);

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Subscribing to status message request");
    Serial.print("\tTopic:\t");
    Serial.println(MQTT_STATUS_REQUEST_TOPIC);
  }
  mqtt.subscribe(MQTT_STATUS_REQUEST_TOPIC);

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Publishing Connected message");
    Serial.print("\tTopic:\t");
    Serial.println(MQTT_CONNECTED_TOPIC);
    Serial.print("\tMessage:\t");
    Serial.println(MQTT_CONNECTED_MESSAGE);
  }
  mqtt.publish(MQTT_CONNECTED_TOPIC, MQTT_CONNECTED_MESSAGE);
}

void setup() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.begin(SERIAL_BAUDRATE);
    while(!Serial) { }
    Serial.println("\n=== setup() ===");
  }
  mqtt.setBufferSize(1024);

  setupWifi();
  setupMqtt();
  digiPot.init();
  pinMode(THERMISTOR_VCC_PIN, OUTPUT);
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Setup Completed");
  }
}

void setupWifi() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Connecting to WiFi");
    printWiFiSettings();
  }

  configureWiFi();

  if(DEBUG_PRINT_SERIAL) {
    Serial.print("\tNew hostname: ");
    Serial.println(WiFi.hostname());
  }

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    if(DEBUG_PRINT_SERIAL) {
      Serial.print(".");
    }
    delay(500);
  }
  
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("\nWiFi Connected");
    Serial.print("\tIP address:\t");
    Serial.println(WiFi.localIP());
    Serial.print("\tRRSI: ");
    Serial.println(WiFi.RSSI());
  }
}

void configureWiFi() {
  WiFi.hostname(WIFI_HOSTNAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void printWiFiSettings() {
    Serial.print("\tMAC:\t");
    Serial.println(WiFi.macAddress());
    Serial.print("\tSSID:\t");
    Serial.println(WIFI_SSID);
    Serial.print("\tDefault hostname: ");
    Serial.println(WiFi.hostname());
    Serial.print("\tWanted Hostname:\t");
    Serial.println(WIFI_SSID);
}

void setupMqtt() {
  mqtt.setClient(wifi);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(1024);
  mqtt.setCallback(mqttDataCallback);
  if(DEBUG_PRINT_SERIAL) {
    printMqttSettings();
  }
  connectMqtt();
}

void printMqttSettings() {
  Serial.println("Connecting to MQTT");
  Serial.print("\tHost:\t");
  Serial.println(MQTT_HOST);
  Serial.print("\tPort:\t");
  Serial.println(MQTT_PORT);
  Serial.print("\tClient ID:\t");
  Serial.println(MQTT_CLIENT_ID);
  Serial.print("\tUsername:\t");
  Serial.println(MQTT_USERNAME);
  Serial.print("\tLast Will Topic:\t");
  Serial.println(MQTT_LAST_WILL_TOPIC);
  Serial.print("\tLast Will Message:\t");
  Serial.println(MQTT_LAST_WILL_MESSAGE);
}

void processNet() {
  if (WiFi.status() == WL_CONNECTED) {
    if (mqtt.connected()) {
      mqtt.loop();
    } else {
      if(DEBUG_PRINT_SERIAL) {
        Serial.println("WiFi Connected but MQTT Not connected. Connecting MQTT");
      }
      connectMqtt();
    }
  } else {
    if (mqtt.connected()){
      if(DEBUG_PRINT_SERIAL) {
        Serial.print("WiFi Disconnected, Also disconnecting MQTT");
      }
      mqtt.disconnect();
    }
  }
  if (!mqtt.connected() && !pendingDisconnect) {
    pendingDisconnect = true;
    if(DEBUG_PRINT_SERIAL) {
      Serial.println("MQTT Disconnected");
    }
  }
}

void connectMqtt() {
  if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_LAST_WILL_TOPIC, 1, false, MQTT_LAST_WILL_MESSAGE)) {
    pendingDisconnect = false;
    mqttConnectedCallback();
  }
}

void loop() {
  processNet();

  if(lastHeartbeatMillis == 0 || (lastHeartbeatMillis + (HEARTBEAT_INTERVAL * 1000)) <= millis()) {
    heartbeat();
    lastHeartbeatMillis = millis();
  }

  if(workNow || lastWorkMillis == 0 || (lastWorkMillis + (workInterval * 1000)) <= millis()) {
    lastWorkMillis = millis();
    workNow = false;
    work();
  }
}

void heartbeat() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("=== heartbeat() ===");
  }
  if(!mqtt.publish(MQTT_HEARTBEAT_TOPIC, "alive")) {
    if(DEBUG_PRINT_SERIAL) {
      Serial.println("Failed to send heartbeat");
    }
  }
}

void work() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("=== work() ===");
  }

  StaticJsonDocument<1024> doc;
  JsonArray metrics = doc.createNestedArray("metrics");

#if ONEWIRE_ENABLED
  sensors.requestTemperatures(); 
  float oneWireTemperature = sensors.getTempCByIndex(0);
  createMetric(metrics, "OneWireTemperature", oneWireTemperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Temperature: ");
    Serial.println(oneWireTemperature);
  }
#endif

  digitalWrite(THERMISTOR_VCC_PIN, turn_On);
  delay(20);
  float analogReading = analogRead(THERMISTORPIN);
  digitalWrite(THERMISTOR_VCC_PIN, turn_Off);
  createMetric(metrics, "AnalogReading", analogReading);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Analog Reading: ");
    Serial.println(analogReading);
  }

  float readingVoltage = analogReading * (VOLTAGE / ADC_MAX);
  createMetric(metrics, "ReadingVoltage", readingVoltage);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Reading Voltage: ");
    Serial.println(readingVoltage);
  }

  float readingResistance = SERIESRESISTOR/(ADC_MAX/analogReading-1);
  createMetric(metrics, "ReadingResistance", readingResistance);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Reading Resistance: ");
    Serial.println(readingResistance);
  }

  float bCoefficientTemperature = bCoefficient(THERMISTORNOMINAL, TEMPERATURENOMINAL, BCOEFFICIENT, readingResistance);
  createMetric(metrics, "BCoefficientTemperature", bCoefficientTemperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("B Coefficient Temperature: ");
    Serial.println(bCoefficientTemperature);
  }

  float steinhartHartTemperature = steinhart.toTemperature(readingResistance);
  createMetric(metrics, "SteinhartHartTemperature", steinhartHartTemperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Steinhart Hart Temperature: ");
    Serial.println(steinhartHartTemperature);
 }

  createMetric(metrics, "Mode", modeNames[currentMode]);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Current Mode: ");
    Serial.println(modeNames[currentMode]);
  }

  createMetric(metrics, "FixedTemp", fixedTemp);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Fixed Temp: ");
    Serial.println(fixedTemp);
  }

  createMetric(metrics, "OffsetTemp", offsetTemp);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("OffsetTemp: ");
    Serial.println(offsetTemp);
  }

  float targetTemperature = calculateTargetTemperature(steinhartHartTemperature);
  createMetric(metrics, "TargetTemperature", targetTemperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Target Temperature: ");
    Serial.println(targetTemperature);
  }

  float targetResistance = steinhart.toResistance(targetTemperature);
  createMetric(metrics, "TargetResistance", targetResistance);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Target Resistance: ");
    Serial.println(targetResistance);
  }

  float digiPotStep = digiPot.calculateSteps(targetResistance);
  createMetric(metrics, "DigiPotStep", digiPotStep);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Digi Pot Step: ");
    Serial.println(digiPotStep);
  }

  float assumedOutResistance = digiPot.calculateAssumedResistance(digiPotStep);
  createMetric(metrics, "AssumedOutResistance", assumedOutResistance);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Assumed output resistance:");
    Serial.println(assumedOutResistance);
  }

  float assumedOutTemperature = steinhart.toTemperature(assumedOutResistance);
  createMetric(metrics, "AssumedOutTemperature", assumedOutTemperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Assumed Out Temperature: ");
    Serial.println(assumedOutTemperature);
  }

  digiPot.writeSteps(digiPotStep);

  String message = "";
  serializeJson(doc, message);
  Serial.print("Sending MQTT message on topic:");
  Serial.println(MQTT_DDATA_TOPIC);
  Serial.print("Payload:");
  Serial.println(message);

  mqtt.publish(MQTT_DDATA_TOPIC, message.c_str());
}

float calculateTargetTemperature(float temperature) {
  float targetTemperature;
  switch(currentMode) {
    case FIXED:
      return fixedTemp;
      break;
    case OFFSET:
      return temperature + offsetTemp;
      break;
    case BYPASS:
    default:
      return temperature;
      break;
  }
}
