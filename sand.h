#ifndef SAND
#define SAND

#include <stdint.h>
#include "globalVars.h"
#include "ws2812.h"

//public functions
void gravity(int32_t angle, int32_t randomDir);
int32_t sandFlow(int32_t angle, float ratio);
void sandToWS2812(int32_t filterBits);

#endif
