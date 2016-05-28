#ifndef GLOBAL_VARS
#define GLOBAL_VARS

//each pixel will show a MULTIPLY x MULTIPLY matrix
#define MULTIPLY 6

#include <stdint.h>
#include "ws2812.h"

//matrix for calculating the sand "physics"
extern volatile int32_t calcMatrix[16 * MULTIPLY][8 * MULTIPLY];

// color of the sand (max brightness)
extern rgb24_t sandCol;

#endif
