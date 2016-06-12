
#include "init.h"

void InitGpioD()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);


	GPIO_InitTypeDef GPIO_InitType;

	GPIO_InitType.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitType.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitType.GPIO_OType = GPIO_OType_PP;
	GPIO_InitType.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitType.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitType);


	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);

	GPIO_InitType.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitType.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOD, &GPIO_InitType);

}

void InitTimer()
{
	TIM_TimeBaseInitTypeDef timerInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	timerInitStructure.TIM_Prescaler = 4000 - 1;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 400 - 1;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &timerInitStructure);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	timerInitStructure.TIM_Prescaler = 42 - 1; //84000000 / 42 = 2000000
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 40000 - 1; //2000000 / 40000 = 50
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &timerInitStructure);
	TIM_Cmd(TIM4, ENABLE);
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

void InitPWM(void)
{
	TIM_OCInitTypeDef TIM_OCStruct;

	/* PWM mode 2 = Clear on compare match */
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM4, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
}

void InitSPI(void)
{
	SPI_InitTypeDef SPI_initStruct;
	SPI_initStruct.SPI_Mode = SPI_Mode_Master;
	SPI_initStruct.SPI_Direction = SPI_Direction_Tx;
	SPI_initStruct.SPI_DataSize = SPI_DataSize_16b;
	SPI_initStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_initStruct.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_initStruct.SPI_NSS = SPI_NSS_Hard;
	//SPI_initStruct.SPI_CPHA = SPI_CPHA_1Edge;
	//SPI_initStruct.SPI_CPOL = SPI_CPOL_High;



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
