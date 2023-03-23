#include "defines.h"

#if defined(ESP32)
#include <EEPROM.h> // https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM
#elif defined(ESP8266)
#include <ESP_EEPROM.h> // https://github.com/jwrw/ESP_EEPROM
#endif

#if ONEWIRE_ENABLED
  #include <OneWire.h>
  #include <DallasTemperature.h>
#endif

#include "MCP_ADC.h" // https://github.com/RobTillaart/MCP_ADC
#include "MCP41X1.h"
#include "thermistor.h"
#include "SparkPlugDevice.h"

MCP41X1 digiPot = MCP41X1(CS_PIN, POT_STEPS, WIPER_RESISTANCE, MAX_RESISTANCE);
MCP3202 mcp1;

#if ONEWIRE_ENABLED
  OneWire oneWire(ONEWIRE_PIN);
  DallasTemperature sensors(&oneWire);
#endif

void deviceCommandCallback(char* payload);
void connectedCallback();

int lastWorkMillis = 0;
int lastHeartbeatMillis = 0;
bool workNow = false;

SteinhartHart steinhart = SteinhartHart(CONST_A, CONST_B, CONST_C);

SparkPlugDevice device;

void deviceCommandCallback(char* payload) {
  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    if(DEBUG_PRINT_SERIAL) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.f_str());
    }
    return;
  }
  JsonArray arr = doc["metrics"];
  for (JsonObject repo : arr) {
    const char* name = repo["name"];
    if(DEBUG_PRINT_SERIAL) {
      Serial.print("Metric name: ");
      Serial.println(name);
    }
    if(strcmp(name, "Mode") == 0) {
      if(strcmp(repo["value"], "FIXED") == 0) {
        StoreData.currentMode = FIXED;
      }
      else if(strcmp(repo["value"], "OFFSET") == 0)
      {
        StoreData.currentMode = OFFSET;
      }
      else if(strcmp(repo["value"], "BYPASS") == 0)
      {
        StoreData.currentMode = BYPASS;
      }
      else if(strcmp(repo["value"], "OFFSET_WITH_MAX") == 0)
      {
        StoreData.currentMode = OFFSET_WITH_MAX;
      }
      if(DEBUG_PRINT_SERIAL) {
        Serial.print("New Mode: ");
        Serial.println(StoreData.currentMode);
      }
    }
    if(strcmp(name, "OffsetTemp") == 0) {
      StoreData.offsetTemp = repo["value"];
      if(DEBUG_PRINT_SERIAL) {
        Serial.print("New OffsetTemp: ");
        Serial.println(StoreData.offsetTemp);
      }
    }
    if(strcmp(name, "FixedTemp") == 0) {
      StoreData.fixedTemp = repo["value"];
      if(DEBUG_PRINT_SERIAL) {
        Serial.print("New FixedTemp: ");
        Serial.println(StoreData.fixedTemp);
      }
    }
  }
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Saving to EEPROM");
  }
  EEPROM.put(EEADDR, StoreData);
  boolean ok = EEPROM.commit();
  if(DEBUG_PRINT_SERIAL) {
    Serial.println((ok) ? "Commit OK" : "Commit failed");
  }
  workNow = true;
}

void setup() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println("\n=== Setup ===");
    Serial.printf("ONEWIRE_PIN:\t%d\n\r", ONEWIRE_PIN);
    Serial.printf("THERMISTORPIN:\t%d\n\r", THERMISTORPIN);
    Serial.printf("ADC_BITS:\t%d\n\r", ADC_BITS);
    Serial.printf("ADC_MAX:\t%.f\n\r", ADC_MAX);

    Serial.printf("MOSI: \t%d\n\r", MOSI);
    Serial.printf("MISO: \t%d\n\r",MISO);
    Serial.printf("SCK: \t%d\n\r", SCK);
    Serial.printf("SS: \t%d\n\r", SS);

    Serial.println("Getting from EEPROM");
  }

  mcp1.selectVSPI();
  mcp1.begin(ADC_CS_PIN);
  mcp1.setSPIspeed(4000000);

  Serial.println("ADC\tCHANNELS\tMAXVALUE");
  Serial.printf("mcp1\t%d\t\t%d\n\r", mcp1.channels(), mcp1.maxValue());

  analogReadResolution(ADC_BITS);

  EEPROM.begin(sizeof(StoreData));
  EEPROM.get(EEADDR, StoreData);

  if(StoreData.currentMode == -1) {
    StoreData.currentMode = BYPASS;
  }
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Mode: ");
    Serial.println(StoreData.currentMode);
  }

  device.setLogging(DEBUG_PRINT_SERIAL);
  device.setDeviceCommandCallback(deviceCommandCallback);
  device.setConnectedCallback(connectedCallback);
  device.setup(
    WIFI_SSID, WIFI_PASSWORD,
    MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD,
    SPARKPLUG_GROUP_ID, SPARKPLUG_NODE_ID, SPARKPLUG_DEVICE_ID
  );

  digiPot.init();
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Setup Completed");
  }
}

void loop() {
  device.loop();

  if(HEARTBEAT_INTERVAL > 0 && (lastHeartbeatMillis == 0 || (lastHeartbeatMillis + (HEARTBEAT_INTERVAL * 1000)) <= millis())) {
    heartbeat();
    lastHeartbeatMillis = millis();
  }

  if(workNow || lastWorkMillis == 0 || (lastWorkMillis + (WORK_INTERVAL * 1000)) <= millis()) {
    lastWorkMillis = millis();
    workNow = false;
    work();
  }
}

void heartbeat() {
  device.heartbeat();
}

void work() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("=== work() ===");
  }

  device.publishDeviceData(readAndAdjust());
}

void connectedCallback() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("=== connected() ===");
  }
  device.publishDeviceBirth(readAndAdjust());  
}

DynamicJsonDocument readAndAdjust() {

  DynamicJsonDocument doc(1024);
  JsonArray metrics = doc.createNestedArray("metrics");

#if ONEWIRE_ENABLED
  sensors.requestTemperatures(); 
  float oneWireTemperature = sensors.getTempCByIndex(0);
  createMetric(metrics, "OneWireTemperature", oneWireTemperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("OneWireTemperature: ");
    Serial.println(oneWireTemperature);
  }
#endif

  // digitalWrite(THERMISTOR_VCC_PIN, turn_On);
  // delay(50);
  // int analogReading = 0;
  // for (int i=0; i < 5; i++) {
  //   analogReading = analogReading + analogRead(THERMISTORPIN);
  // }
  // analogReading = analogReading/5;
 
  // digitalWrite(THERMISTOR_VCC_PIN, turn_Off);
  uint16_t analogReading = mcp1.analogRead(0);

  createMetric(metrics, "AnalogReading", analogReading);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Analog Reading: ");
    Serial.println(analogReading);
  }

  if(analogReading == ADC_MAX) {
    if(DEBUG_PRINT_SERIAL) {
      Serial.println("No thermistor connected.");
    }
    return doc;
  }

  float readingVoltage = (analogReading * (VOLTAGE / ADC_MAX));
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

  float correctedResistance = readingResistance + RESISTANCE_CORRECTION;
  createMetric(metrics, "CorrectedResistance", correctedResistance);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Corrected Resistance: ");
    Serial.println(correctedResistance);
  }

  float bCoefficientTemperature = bCoefficient(THERMISTORNOMINAL, TEMPERATURENOMINAL, BCOEFFICIENT, correctedResistance);
  createMetric(metrics, "BCoefficientTemperature", bCoefficientTemperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("B Coefficient Temperature: ");
    Serial.println(bCoefficientTemperature);
  }

  float steinhartHartTemperature = steinhart.toTemperature(correctedResistance);
  createMetric(metrics, "SteinhartHartTemperature", steinhartHartTemperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Steinhart Hart Temperature: ");
    Serial.println(steinhartHartTemperature);
  }

  float temperature = steinhartHartTemperature + TEMPERATURE_CORRECTION;
  createMetric(metrics, "Temperature", temperature);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Temperature: ");
    Serial.println(temperature);
  }

  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Current Mode: ");
    Serial.println(modeNames[StoreData.currentMode]);
  }
  createMetric(metrics, "Mode", modeNames[StoreData.currentMode]);

  createMetric(metrics, "FixedTemp", StoreData.fixedTemp);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Fixed Temp: ");
    Serial.println(StoreData.fixedTemp);
  }

  createMetric(metrics, "OffsetTemp", StoreData.offsetTemp);
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("OffsetTemp: ");
    Serial.println(StoreData.offsetTemp);
  }

  float targetTemperature = calculateTargetTemperature(temperature);
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

  return doc;
}

float calculateTargetTemperature(float temperature) {
  float targetTemperature;
  switch(StoreData.currentMode) {
    case FIXED:
      return StoreData.fixedTemp;
    case OFFSET:
      return temperature + StoreData.offsetTemp;
    case OFFSET_WITH_MAX:
      if(temperature + StoreData.offsetTemp >= StoreData.fixedTemp) {
        return StoreData.fixedTemp;
      } else {
        return temperature + StoreData.offsetTemp;
      }
    case BYPASS:
    default:
      return temperature;
  }
}
