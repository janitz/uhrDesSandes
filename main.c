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

int32_t angle = 0;
int32_t state = 0;
float ratio = 1;
int32_t filter = 2;
int32_t anim_count = 0;

int32_t calcOrNot = 0;

int main(void)
{
	uint32_t n;

	initMatrix();
	SystemInit(); // init quarz settings

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

	sandCol.b = 0;
	sandCol.g = 30;
	sandCol.r = 40;

	for (n = 0; n < WS2812_CHAIN_LEN; n++)
	{
		WS2812_LED_BUF[n] = col1;
	}

	// start dma
	ws2812_refresh();


    while(1)
    {


    }
}

void TIM2_IRQHandler()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
    	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    	GPIOD->ODR ^= GPIO_Pin_15;//if the blue led is blinking the interrupt works :)

/*
    	int32_t j, x, y;
        for (y = 0; y < (16 * MULTIPLY) + (8 * MULTIPLY); ++y) {
			for (x = 0; x < 8 * MULTIPLY; ++x) {
				j = y + x - (8 * MULTIPLY);
				if (j < 16 * MULTIPLY && j >= 0)
				{
					calcMatrix[j][x] = 0;
					if(y == anim_count / 8) calcMatrix[j][x] = 1;
				}
			}
		}

        anim_count++;
        if ((anim_count / 8) > (16 * MULTIPLY) + (8 * MULTIPLY)) anim_count = 0;
*/

        switch (state) {
			case 0:
				TIM4->CCR1 = 2000;
				ratio -= 0.0025;
				filter++;
				if (filter > 24){filter = 24;}
				if (ratio < 0)
				{
					ratio = 0;
					state = 1;
					anim_count = 0;
				}
				break;
			case 1:
				TIM4->CCR1 = 4000;
				if (anim_count < 50)
				{
					anim_count ++;
					break;
				}
				angle += 1;
				if (angle > 155)
				{
					ratio += 0.00125;
				}
				filter--;
				if (filter < 4){filter = 4;}
				if (angle > 180)
				{
					angle = 180;
					state = 2;
					anim_count = 0;
				}
				break;
			case 2:
				TIM4->CCR1 = 2000;
				ratio += 0.0025;
				filter++;
				if (filter > 24){filter = 24;}
				if (ratio > 1)
				{
					ratio = 1;
					state = 3;
					anim_count = 0;
				}
				break;
			case 3:
				TIM4->CCR1 = 4000;
				if (anim_count < 50)
				{
					anim_count ++;
					break;
				}
				angle -= 1;
				if (angle < 25)
				{
					ratio -= 0.00125;
				}
				filter--;
				if (filter < 4){filter = 4;}
				if (angle < 0)
				{
					angle = 0;
					state = 0;
					anim_count = 0;
				}
				break;
			default:
				angle = 0;
				state = 0;
				ratio = 1;
				anim_count = 0;
				filter = 2;
				break;
		}

        calcOrNot ++;
        if(calcOrNot > 1)
        {
        	calcOrNot = 0;
        }

        if(calcOrNot == 0)
        {
        	gravity(angle, -1);
			sandFlow(angle, ratio);
			gravity(angle, 0);
        }
        sandToWS2812(filter / 2);

    	ws2812_refresh();




    }
}


