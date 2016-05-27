#ifndef INIT
#define INIT

#include <stdint.h>
#include <stm32f4xx_conf.h>
#include "globalVars.h"

volatile int32_t calcMatrix[16 * MULTIPLY][8 * MULTIPLY];

uint32_t anim_count;

void InitGpioD();
void InitTimer();
void EnableTimerInterrupt();
void initMatrix();

#endif
