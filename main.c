/*
 * Project    : Hourglas
 * Author     : Janis, Lars
 * Date       : 26.3.2016
 * MCU        : STM32F407VGT Discovery Board
 * IDE        : CooCox CoIDE 1.7.7
 */

// the libraries are from http://mikrocontroller.bplaced.net/wordpress/?page_id=744





#include <stdint.h>
#include <stm32f4xx_conf.h>

#include "init.h"
#include "ws2812.h"
#include "globalVars.h"
#include "sand.h"

rgb24_t sandCol;


//D12-D15 std LEDS


void calcMatrixToDmaBuffer();

int main(void)
{
	uint32_t n;

	initMatrix();
	SystemInit(); // init quarz settings

	InitGpioD();
	InitTimer();
	EnableTimerInterrupt();


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

#ifdef foooo
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
#endif

        anim_count++;

        gravity(anim_count*5);
        calcMatrixToDmaBuffer();
        ws2812_refresh();
    }
}


void calcMatrixToDmaBuffer()
{
	rgb24_t outCol;
	int32_t i, j, x, y;
	uint32_t r, g, b;

	int32_t reversed; //bool
	int32_t ledNr;

	//rows of the led matix
	for (y = 0; y < 16; ++y)
	{
		//the led matrix is a serpentine, so every second line is reversed
		reversed = y % 2;

		//columns  of the led matix
		for (x = 0; x < 8; ++x)
		{
			r = 0;
			g = 0;
			b = 0;

			//each led pixel has a MULTIPLY by MULTIPLY field
			//of corresponding sand parts in the calc matrix
			for (j = 0; j < MULTIPLY; ++j)
			{
				for (i = 0; i < MULTIPLY; ++i)
				{
					//if theres sand in the field
					if ((calcMatrix[(y * MULTIPLY) + j][(x * MULTIPLY) + i] == 1))
					{
						r += sandCol.r;
						g += sandCol.g;
						b += sandCol.b;
					}
				}
			}

			//divide to get the average brightness
			outCol.r = r / (MULTIPLY * MULTIPLY);
			outCol.g = g / (MULTIPLY * MULTIPLY);
			outCol.b = b / (MULTIPLY * MULTIPLY);

			//calculate the led nr
			ledNr = y * 8;
			if (reversed)
			{
				ledNr += 7 - x;
			}
			else
			{
				ledNr += x;
			}

			//save the color to the array used by the dma
			WS2812_LED_BUF[ledNr] = outCol;
		}
	}
}
