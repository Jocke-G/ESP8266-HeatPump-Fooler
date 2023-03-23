#include "MCP41X1.h"

MCP41X1::MCP41X1(int chipSelectPin, float steps, float wiperResistance, float maxResistance) {
  this->chipSelectPin = chipSelectPin;
  this->steps = steps;
  this->wiperResistance = wiperResistance;
  this->maxResistance = maxResistance;

  this->resistancePerStep = calculateResistancePerStep();
}

void MCP41X1::init() {
  pinMode(this->chipSelectPin, OUTPUT);
  SPI.begin();
}

float MCP41X1::getResistancePerStep() {
  return this->resistancePerStep;
}

float MCP41X1::calculateResistancePerStep() {
  return (this->maxResistance-this->wiperResistance) / (this->steps-1);
}

int MCP41X1::calculateSteps(float resistance) {
  if(resistance <= this->wiperResistance)
    return 0;

  if(resistance >= this->maxResistance)
    return this->steps-1;

  return round((resistance - this->wiperResistance) / this->resistancePerStep);
}

float MCP41X1::calculateAssumedResistance(int steps) {
  return this->wiperResistance + (this->resistancePerStep * steps);
}

void MCP41X1::writeSteps(int value) {
  digitalWrite(this->chipSelectPin, LOW);
  SPI.transfer(COMMAND_WRITE);
  SPI.transfer(value);
  digitalWrite(this->chipSelectPin, HIGH);
}

void MCP41X1::writeResistance(float resistance) {
  int steps = calculateSteps(resistance);
  writeSteps(steps);
}
