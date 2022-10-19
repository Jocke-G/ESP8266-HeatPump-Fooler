#ifndef THERMISTOR_H
#define THERMISTOR_H

#define KELVIN_FREEZING 273.15

#include <Arduino.h>

class SteinhartHart {
  private:
    float a;
    float b;
    float c;

  public:
    SteinhartHart(float a, float b, float c);
    float toTemperature(float resistance);
    float toResistance(float temp);
};

float steinhartHart(float a, float b, float c, float resistance);
float reverseSteinhartHart(float a, float b, float c, float temp);

float bCoefficient(int thermistorNominal, int temperatureNominal, float bCoefficient, float resistance);

#endif
