#ifndef SPARKPLUG_B_H
#define SPARKPLUG_B_H

#include <Arduino.h>
#include <ArduinoJson.h>

void createMetric(JsonArray metrics, String name, float value);
void createMetric(JsonArray metrics, String name, String value);

#endif
