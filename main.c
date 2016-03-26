/*
 * Project    : Hourglas
 * Author     : Janis, Lars
 * Date       : 26.3.2016
 * MCU        : STM32F407VGT Discovery Board
 * IDE        : CooCox CoIDE 1.7.7
 */


#define HSE_VALUE    ((uint32_t)8000000)

#include <stm32f4xx_conf.h>
#include "stm32_ub_ws2812_8ch.h" //http://mikrocontroller.bplaced.net/wordpress/?page_id=744

//D12-D15 std LEDS



int main(void)
{
	SystemInit(); // init quarz settings

	//enable clocks needed
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	//init PORT_D
	GPIO_InitTypeDef GPIO_InitType;
	GPIO_InitType.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitType.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitType.GPIO_OType = GPIO_OType_PP;
	GPIO_InitType.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitType.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitType);

	// init WS2812-chains
	UB_WS2812_Init();
	WS2812_CHAIN_LEN.ch1=64;

	// create color
	WS2812_RGB_t col;
	col.blue=0;
	col.green=10;
	col.red=0;



	// set all leds to col
	UB_WS2812_All_RGB(0xFF, col);


	// start dma
	UB_WS2812_RefreshAll();





	int x;

    while(1)
    {

		GPIOD->ODR ^= GPIO_Pin_12;
		for(x = 0 ; x < 5000000; x++){}

		GPIOD->ODR ^= GPIO_Pin_13;
    	for(x = 0 ; x < 5000000; x++){}

    	GPIOD->ODR ^= GPIO_Pin_14;
    	for(x = 0 ; x < 5000000; x++){}

    	GPIOD->ODR ^= GPIO_Pin_15;
		for(x = 0 ; x < 5000000; x++){}
    }
}
