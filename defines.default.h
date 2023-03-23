// WiFi
#define WIFI_SSID "***"
#define WIFI_PASSWORD "***"

// MQTT
#define MQTT_HOST "1.2.3.4"
#define MQTT_PORT 1883
#define MQTT_USERNAME "***"
#define MQTT_PASSWORD "***"

// Sparkplug_B
#define SPARKPLUG_GROUP_ID "Fredriksberg"
#define SPARKPLUG_NODE_ID "HeatpumpFooler"
#define SPARKPLUG_DEVICE_ID "ThermiaVillaClassic"

// Work intervals
#define WORK_INTERVAL 60 * 5 // Seconds
#define HEARTBEAT_INTERVAL 0 // Seconds. 0 = disabled

// Arduino board
#define ADC_BITS 10
#define THERMISTORPIN A0 // ESP8266: A0 / ESP32: 34
#define VOLTAGE 3.3

// Debug
#define SERIAL_BAUDRATE 115200
#define DEBUG_PRINT_SERIAL true

// Thermistor
#define THERMISTORNOMINAL 150
#define TEMPERATURENOMINAL 25
#define BCOEFFICIENT 3335.46
#define SERIESRESISTOR 1917
#define RESISTANCE_CORRECTION 0
#define TEMPERATURE_CORRECTION 0

// Steinhart–Hart coefficients
#define CONST_A 1.951203678e-3
#define CONST_B 2.747041896e-4
#define CONST_C 2.096215130e-7

// MCP3XXX Analog Digital Converter
#define ADC_CS_PIN 5 // ADC Chip Select pin. ESP8266: ? / ESP32: 5

// MCP41X1 Digital Pot
#define CS_PIN D8 // Pot Chip Select pin. ESP8266: D8 / ESP32: 17
#define POT_STEPS 256
#define MAX_OHMS 5000
#define WIPER_RESISTANCE 75

// OneWire
#define ONEWIRE_ENABLED false
#define ONEWIRE_PIN D3 // . ESP8266: D3 / ESP32: 4

// Science
#define KELVIN_FREEZING 273.15
#define ADC_MAX float(pow(2, ADC_BITS) - 1)

enum MODE {
  UNKNOWN,
  BYPASS,
  OFFSET,
  FIXED,
  OFFSET_WITH_MAX
};
const char* modeNames[] = { "UNKNOWN", "BYPASS", "OFFSET", "FIXED", "OFFSET_WITH_MAX" };

#define EEADDR 0

struct StoreData_s {
  MODE currentMode;
  int offsetTemp;
  int fixedTemp;
};

StoreData_s StoreData = {
  BYPASS,
  5,
  16
};
