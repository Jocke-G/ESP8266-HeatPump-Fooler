#ifndef MCP41X1_H
#define MCP41X1_H

#include <Arduino.h>
#include <SPI.h>

#define COMMAND_WRITE 0x00

class MCP41X1 {
  private:
    int chipSelectPin;
    float maxResistance;
    float resistancePerStep;
    float steps;
    float wiperResistance;

    float calculateResistancePerStep();

  public:

    MCP41X1(int chipSelectPin, float steps, float wiperResistance, float maxResistance);
    void init();
    int calculateSteps(float resistance);
    float calculateAssumedResistance(int steps);
    void writeResistance(float resistance);
    void writeSteps(int value);
    float getResistancePerStep();
};

#endif
