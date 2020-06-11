// ! Pressure Sensing ! ========================================================

#pragma once

#include <Adafruit_MPRLS.h>
#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))  // Bit checking macro
#define PRESSURE_RST -1                             // Hard reset on begin()
#define PRESSURE_EOC -1                             // End-of-conversion

Adafruit_MPRLS sensor = Adafruit_MPRLS(PRESSURE_RST, PRESSURE_EOC);
pressure       cmH20;

typedef struct {
    float
        atmosphere,
        airway,
        peep,
        peak;
} pressure;

// check if airway pressure is over max
bool pressureOver();

// check if airway pressure is high
bool pressureHigh();

// return the current airway pressure
float getAirway();

// current sensor pressure reporting in cmH20
float getPressure();

// read from sensor state bits to check health of sensor
void pressureSensorCheck();

// check if sensor is ready to be read
bool pressureSensorReady();
