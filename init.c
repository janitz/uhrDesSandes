
#include "init.h"

void InitGpioD()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef GPIO_InitType;
	GPIO_InitType.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitType.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitType.GPIO_OType = GPIO_OType_PP;
	GPIO_InitType.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitType.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitType);
}

void InitTimer()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef timerInitStructure;
	timerInitStructure.TIM_Prescaler = 4000;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 400;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &timerInitStructure);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void EnableTimerInterrupt()
{
    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

void initMatrix()
{
	int32_t y = 0;
	int32_t x = 0;
	int32_t y2 = 0;
	int32_t x2 = 0;
	int32_t maskVal = -1;

	for (y = 0; y < 16 * MULTIPLY; ++y)
	{
		for (x = 0; x < 8 * MULTIPLY; ++x)
		{

			calcMatrix[y][x] = 0;
		}
	}

	for (y = 0; y < MULTIPLY; ++y)
	{
		for (x = 0; x < MULTIPLY; ++x)
		{
			for (y2 = 0; y2 < 3; ++y2)
			{
				for (x2 = 0; x2 <= y2; ++x2)
				{
					calcMatrix[y + ((2 - y2) * MULTIPLY)][x + (x2 * MULTIPLY)] = maskVal;
					calcMatrix[y + ((2 - y2) * MULTIPLY)][x + ((7 - x2) * MULTIPLY)] = maskVal;
					calcMatrix[y + ((5 + y2) * MULTIPLY)][x + ((7 - x2) * MULTIPLY)] = maskVal;

					calcMatrix[y + ((10 - y2) * MULTIPLY)][x + (x2 * MULTIPLY)] = maskVal;
					calcMatrix[y + ((13 + y2) * MULTIPLY)][x + (x2 * MULTIPLY)] = maskVal;
					calcMatrix[y + ((13 + y2) * MULTIPLY)][x + ((7 - x2) * MULTIPLY)] = maskVal;

				}
			}
		}
	}

	int32_t count = 0;
	for (y = 0; y < 8 * MULTIPLY; ++y)
	{
		for (x = 0; x < 8 * MULTIPLY; ++x)
		{
			if (calcMatrix[y][x] == 0 && count < 23 * MULTIPLY * MULTIPLY)
			{
				count++;
				calcMatrix[y][x] = 1;
			}
		}
	}

}
