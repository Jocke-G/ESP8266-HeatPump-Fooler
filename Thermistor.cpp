#include "thermistor.h"

float bCoefficient(int thermistorNominal, int temperatureNominal, float bCoefficient, float resistance) {
  return 1.0 /((log(resistance / thermistorNominal) / bCoefficient) + 1.0 / (temperatureNominal + KELVIN_FREEZING)) - KELVIN_FREEZING;
}

SteinhartHart::SteinhartHart(float a, float b, float c) {
  this->a = a;
  this->b = b;
  this->c = c;
}

float SteinhartHart::toTemperature(float resistance) {
  return steinhartHart(this->a, this->b, this->c, resistance);
}

float SteinhartHart::toResistance(float temp) {
  return reverseSteinhartHart(this->a, this->b, this->c, temp);
}

float steinhartHart(float a, float b, float c, float resistance) {
  return 1 / (a + b * log(resistance) + c * pow(log(resistance), 3)) - KELVIN_FREEZING;
}

float reverseSteinhartHart(float a, float b, float c, float temp) {
  float x = (a - 1/(temp + KELVIN_FREEZING))/c;
  float B_C = b/c;
  float y = sqrt(B_C*B_C*B_C/27 + x*x/4);

  return exp(pow(y - x/2,1/3.) - pow(y + x/2,1/3.));
}
