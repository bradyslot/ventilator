// ! Handy Dandy stuff ! =======================================================

#pragma once

// floating point re-implementation of map()
float mapf(float x, float in_min, float in_max, float out_min, float out_max);

// floating point comparison can fail by rounding tolerances
bool roughlyEqual(float a, float b);
