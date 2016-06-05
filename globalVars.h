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


extern int32_t calcOrNot;

#endif


/*
 *calcMatrix:
 *                  X X X O O X X X
 *                  X X O O O O X X
 *                  X O O O O O O X
 *                  O O O O O O O O
 *                  O O O O O O O O
 *                  O O O O O O O X
 *                  O O O O O O X X
 *                  O O O O O X X X
 *  X X X O O O O O
 *  X X O O O O O O
 *  X O O O O O O O
 *  O O O O O O O O
 *  O O O O O O O O
 *  X O O O O O O X
 *  X X O O O O X X
 *  X X X O O X X X
 */
