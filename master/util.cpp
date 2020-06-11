// ! Implementation of utils ! =================================================

#include "util.h"

bool roughlyEqual(float a, float b) {
    if (abs(a - b) < 0.0001)
        return true;
    else
        return false;
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    float v = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    return v;
}