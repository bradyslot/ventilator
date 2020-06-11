// ! Pressure Sensing Implementaiton ! =========================================

#include "pressure.h"

bool pressureOver() {
    if (getAirway() > MAX_PEAK)
        return true;
    else
        return false;
}

bool pressureHigh() {
    if (getAirway() > HIGH_PEAK)
        return true;
    else
        return false;
}

float getAirway() {
    return getPressure() - cmH20.atmosphere;
}

float getPressure() {
    float p;
    p = sensor.readPressure();
    return p * 1.0197;
}

void pressureSensorCheck() {
    uint8_t status = sensor.readStatus();

    // refer to honeywell mpr series datasheet for more details
    bool sensorMathSaturation  = CHECK_BIT(status, 0);  // 1 = saturated
    bool sensorMemoryIntegrity = CHECK_BIT(status, 2);  // 1 = failed
    bool sensorBusy            = CHECK_BIT(status, 5);  // 1 = busy
    bool sensorPower           = CHECK_BIT(status, 6);  // 1 = powered

    Serial.println("Saturated: " + String(sensorMathSaturation));
    Serial.println("Memory Check Failed: " + String(sensorMemoryIntegrity));
    Serial.println("Busy: " + String(sensorBusy));
    Serial.println("Powered: " + String(sensorPower));
}

bool pressureSensorReady() {
    return !CHECK_BIT(sensor.readStatus(), 5);
}