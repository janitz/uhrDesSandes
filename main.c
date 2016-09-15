/*
 * Project    : Hourglas
 * Author     : Janis, Lars
 * Date       : 26.3.2016
 * MCU        : STM32F407VGT Discovery Board
 * IDE        : CooCox CoIDE 1.7.7
 */

// note:
// D12-D15 are the std LEDS on the discovery board

#include <stdint.h>
#include <stm32f4xx_conf.h>

#include "globalVars.h"
#include "init.h"
#include "ws2812.h"
#include "sand.h"

rgb24_t sandCol;

int32_t angle[100] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile int32_t state = -1;
volatile int32_t buttoncolor = -1;
float ratio = 1;
int32_t filter = 4;
int32_t anim_count = 0;

int32_t calcOrNot = 0;

void setServo(int32_t angle);
void handleButton(uint32_t nr);

void sendSettings(uint8_t adress, uint8_t data)
{
	uint32_t cnt;
	uint32_t i;

	//select low
	for (i = 0; i < 100; ++i)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	}


	for(cnt = 0; cnt < 8; cnt++)
	{
		if(adress & (128 >> cnt))
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_7);
		}
		else
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_7);
		}

		for (i = 0; i < 100; ++i)
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_5);
		}
		for (i = 0; i < 100; ++i)
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_5);
		}

	}

	for(cnt = 0; cnt < 8; cnt++)
		{
			if(data & (128 >> cnt))
			{
				GPIO_SetBits(GPIOA, GPIO_Pin_7);
			}
			else
			{
				GPIO_ResetBits(GPIOA, GPIO_Pin_7);
			}

			for (i = 0; i < 100; ++i)
			{
				GPIO_SetBits(GPIOA, GPIO_Pin_5);
			}
			for (i = 0; i < 100; ++i)
			{
				GPIO_ResetBits(GPIOA, GPIO_Pin_5);
			}

		}



	//select high
	for (i = 0; i < 100; ++i)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_4);
	}


}

int main(void)
{
	uint32_t n;

	SystemInit(); // init quarz settings

	initMatrix();

	for (n = 0; n < 20; n++)
	{
		gravity(0, -1);
	}


	InitGpioD();
	InitTimer();
	EnableTimerInterrupt();
	InitPWM();

	// init WS2812-chains
	ws2812_init();

	// create color
	rgb24_t col1;
	col1.b = 10;
	col1.g = 0;
	col1.r = 0;

	sandCol.r = 40; // 40
	sandCol.g = 30; // 30
	sandCol.b = 0;

	for (n = 0; n < WS2812_CHAIN_LEN; n++)
	{
		WS2812_LED_BUF[n] = col1;
	}

	// send a few settings to the ledmatrix driver max 7219
	sendSettings(12, 0);  // shutdownmode on
	sendSettings(9, 0);  // no decode
	sendSettings(10, 5);  // intensity
	sendSettings(11, 7);  // 8 columns to scan
	sendSettings(12, 1);  // shutdownmode off
	sendSettings(13, 0);  // displaytest off


    while(1)
    {
    	//int xx = 10000;
    	//while(xx--);
    	//GPIOD->ODR ^= GPIO_Pin_13;


    	if (buttoncolor)
    	{
    		sendSettings(1, 0); // rot 16 + 32 + 64 + 128
			sendSettings(2, 0); // blau
			sendSettings(3, 240); // grün
    	}
    	else
    	{
    		sendSettings(1, 240); // rot 16 + 32 + 64 + 128
			sendSettings(2, 0); // blau
			sendSettings(3, 0); // grün
    	}

    }
}

void TIM2_IRQHandler()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
    	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    	//if the blue led is blinking the interrupt works :)
    	GPIOD->ODR ^= GPIO_Pin_15;

    	//debounce
    	if(debounceCount > 0){debounceCount--;}

    	int32_t n;
    	for(n = 0; n < 99; n++)
    	{
    		angle[n + 1] = angle[n];
    	}


    	if(angle[99] > 120 && ratio < 1)
    	{
    		ratio += 0.0021; //10s
    	}
    	if(angle[99] < 60 && ratio > 0)
		{
			ratio -= 0.0021; //10s
		}


    	switch (state) {
			case 0:
				if (angle[0] > 0){angle[0] -= 2;}
				break;
			case 1:
				if (angle[0] < 180){angle[0] += 2;}
				break;
			default:
				angle[0] = 0;
				state = 0;
				ratio = 0;
				break;
		}




    	gravity(angle[99], -1);
    	gravity(angle[99], 0);
    	buttoncolor = sandFlow(angle[99], ratio);
        sandToWS2812(filter / 2);

    	ws2812_refresh();



    	setServo(angle[0]);



    }
}

void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		EXTI_ClearITPendingBit(EXTI_Line0);
		if(debounceCount == 0)
		{
			debounceCount = DEBOUNCE_VAL;
			handleButton(0);
		}
	}
}

void EXTI1_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
		EXTI_ClearITPendingBit(EXTI_Line1);
		if(debounceCount == 0)
		{
			debounceCount = DEBOUNCE_VAL;
			handleButton(1);
		}
	}
}

void EXTI2_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line2) != RESET) {
		EXTI_ClearITPendingBit(EXTI_Line2);
		if(debounceCount == 0)
		{
			debounceCount = DEBOUNCE_VAL;
			handleButton(2);
		}
	}
}

void EXTI3_IRQHandler(void) {
	if (EXTI_GetITStatus(EXTI_Line3) != RESET) {
		EXTI_ClearITPendingBit(EXTI_Line3);
		if(debounceCount == 0)
		{
			debounceCount = DEBOUNCE_VAL;
			handleButton(3);
		}
	}
}

void handleButton(uint32_t nr)
{
	GPIOD->ODR ^= GPIO_Pin_14;//the red led toggle
	state = ~state & 1;
}

void setServo(int32_t angle) // 0-180
{
	int32_t cycle = 20000; //us
	int32_t cw = 40000 - 1; //continuous wave

	int32_t zero = cw * 920 / cycle;     // %
	int32_t hundred = cw * 2130 / cycle;  // %

	TIM4->CCR1 = zero + ((hundred - zero) * angle / 180);
}




