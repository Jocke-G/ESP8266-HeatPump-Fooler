// WiFi
#define WIFI_SSID "***"
#define WIFI_PASSWORD "***"
#define WIFI_HOSTNAME "HeatpumpFooler"

// MQTT
#define MQTT_HOST "1.2.3.4"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "HeatpumpFooler"
#define MQTT_USERNAME "***"
#define MQTT_PASSWORD "***"
#define MQTT_CONNECTED_TOPIC "HeatpumpFooler/DBIRTH"
#define MQTT_CONNECTED_MESSAGE "true"
#define MQTT_LAST_WILL_TOPIC "HeatpumpFooler/DDEATH"
#define MQTT_LAST_WILL_MESSAGE "false"
#define MQTT_STATUS_REQUEST_TOPIC "HeatpumpFooler/TechnicalStatusRequest"
#define MQTT_STATUS_RESPONSE_TOPIC "HeatpumpFooler/TechnicalStatus"
#define MQTT_DDATA_TOPIC "HeatpumpFooler/DDATA" // Device Data
#define MQTT_DCMD_TOPIC "HeatpumpFooler/DCMD" // Device Command
#define MQTT_HEARTBEAT_TOPIC "HeatpumpFooler/HEARTBEAT"

#define WORK_INTERVAL 60 * 5 // Seconds
#define HEARTBEAT_INTERVAL 10 // Seconds

// Arduino board
#define ADC_BITS 10
#define THERMISTORPIN A0
#define VOLTAGE 3.3
#define SERIAL_BAUDRATE 115200
#define SERIESRESISTOR 1917
#define turn_On 0
#define turn_Off 1

// Thermistor
#define THERMISTORNOMINAL 150
#define TEMPERATURENOMINAL 25
#define BCOEFFICIENT 3335.46
#define THERMISTOR_VCC_PIN D0

// Steinhart–Hart coefficients
#define A 1.951203678e-3
#define B 2.747041896e-4
#define C 2.096215130e-7

// MCP41X1 Digital Pot
#define CS_PIN D8 // Chip Select pin
#define POT_STEPS 256
#define MAX_OHMS 5060
#define WIPER_RESISTANCE 110

// OneWire
#define ONEWIRE_ENABLED false
#define ONEWIRE_PIN D3

// Science
#define KELVIN_FREEZING 273.15
#define ADC_MAX float(pow(2, ADC_BITS) - 1)

// Debug
#define SERIAL_BAUDRATE 115200
#define DEBUG_PRINT_SERIAL true

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
