#include "SparkPlug_B.h"

void createMetric(JsonArray metrics, String name, float value) {
  JsonObject metric  = metrics.createNestedObject();
  metric["name"]=name;
  metric["value"]=value;
}

void createMetric(JsonArray metrics, String name, String value) {
  JsonObject metric  = metrics.createNestedObject();
  metric["name"]=name;
  metric["value"]=value;
}
