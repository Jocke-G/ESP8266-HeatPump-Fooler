# ESP8266 Heat Pump Fooler

Simulates the outdoor sensor, to make the heat pump work more when electricity is cheaper.

IMPORTANT!
This project is a work in progress! It is not done, it is not perfect.
This project comes with NO WARRANTY OF ANY KIND

## Bill of Material

Wemos D1 Mini (ESP8266)
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

https://www.arduino.cc/reference/en/libraries/esp_eeprom/
https://github.com/jwrw/ESP_EEPROM
