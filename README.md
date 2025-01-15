# ESP8266 Heat Pump Fooler

Simulates the outdoor sensor, to make the heat pump work more when electricity is cheaper.

IMPORTANT!
This project is a work in progress! It is not done, it is not perfect.
This project comes with NO WARRANTY OF ANY KIND

## Bill of Material

Wemos D1 Mini (ESP8266) OR [ESP32](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html)

Flash using board `ESP32 Wrover Kit (all versions)`. NOTE: Did have to rollback to ESP32 version 2.0.17, or net library crash.

NTC 150 Thermistor
MCP4151-502E/P Digital Potentiometer

## Sparkplug_B

https://www.eclipse.org/tahu/spec/Sparkplug%20Topic%20Namespace%20and%20State%20ManagementV2.2-with%20appendix%20B%20format%20-%20Eclipse.pdf

### Device Command

Topic: HeatpumpFooler/DCMD

    {
    "metrics": [
        {
            "name": "Mode",
            "value": "FIXED"
        },
        {
            "name": "_ValidModes",
            "value": "FIXED|OFFSET|BYPASS"
        },
        {
            "name": "OffsetTemp",
            "value": -10
        },
        {
            "name": "FixedTemp",
            "value": 20
        },
        {
            "name":"WorkInterval",
            "value: 60
        }
    ]
    }

## Foregoing Proof of Concepts

[ESP8266 - Proof of Concept - NTC Thermistor](https://github.com/Jocke-G/ESP8266-PoC-NTC-Thermistor)

[ESP8266 - Proof of Concept - MCP41X1 Digital Potentiometer](https://github.com/Jocke-G/ESP8266-PoC-MCP41X1)

## Libraries

[ArduinoJson.h](https://www.arduino.cc/reference/en/libraries/arduinojson/) [Official Webpage](https://arduinojson.org/)

[PubSubClient.h](https://www.arduino.cc/reference/en/libraries/pubsubclient/) [Git repo](https://github.com/knolleary/pubsubclient) [Official Webpage](https://pubsubclient.knolleary.net/api)

[MCP_ADC by RobTillaart](https://github.com/RobTillaart/MCP_ADC) 0.5.1

https://www.arduino.cc/reference/en/libraries/esp_eeprom/
https://github.com/jwrw/ESP_EEPROM

## To Do

### Persistent memory

For ESP8266, [ESP_EEPROM](https://github.com/jwrw/ESP_EEPROM) is used. For ESP32, [EEPROM](https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM) is used, which has a compatible same signarure. But, ESP32 EEPROM is deprecated, replaced by [Preferences](https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences). Find out what to do.

### OTA
