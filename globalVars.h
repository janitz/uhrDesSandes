#ifndef GLOBAL_VARS
#define GLOBAL_VARS

#define MULTIPLY 5

#include <stdint.h>
#include "ws2812.h"

extern volatile int32_t calcMatrix[16 * MULTIPLY][8 * MULTIPLY];

extern rgb24_t sandCol;

extern uint32_t anim_count;


#endif
