#ifndef INIT
#define INIT

#include <stdint.h>
#include <stm32f4xx_conf.h>
#include "globalVars.h"

volatile int32_t calcMatrix[16 * MULTIPLY][8 * MULTIPLY];
volatile int32_t debounceCount;


void InitGpioD();
void InitTimer();
void EnableTimerInterrupt();
void InitPWM(void);
void initMatrix();


#endif
