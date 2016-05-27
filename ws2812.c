#include "ws2812.h"

//from http://mikrocontroller.bplaced.net/wordpress/?page_id=4204

uint32_t ws2812_dma_status;
uint16_t WS2812_TIMER_BUF[WS2812_TIMER_BUF_LEN];

//private function
void p_ws2812_init_io();
void p_ws2812_init_timer();
void p_ws2812_clear_buf();
void p_ws2812_init_NVIC();
void p_ws2812_init_DMA();
void p_ws2812_DMA_start();
void p_ws2812_calc_timer_buf();

void ws2812_init()
{
	uint32_t n;

	ws2812_dma_status=0;

	p_ws2812_clear_buf();

	//alle leds aus
	for(n = 0; n < WS2812_CHAIN_LEN; n++)
	{
		WS2812_LED_BUF[n] = RGB_BLACK;
	}

	// init vom GPIO
	p_ws2812_init_io();
	// init vom Timer
	p_ws2812_init_timer();
	// init vom NVIC
	p_ws2812_init_NVIC();
	// init vom DMA
	p_ws2812_init_DMA();

	// einmal DMA starten (ohne Signal)
	p_ws2812_clear_buf();

	// warte bis DMA-Transfer fertig
	while(ws2812_dma_status!=0);
	// DMA starten
	p_ws2812_DMA_start();

}

void p_ws2812_init_io()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// enable clock
	RCC_AHB1PeriphClockCmd(WS2812_CLOCK, ENABLE);

	// Config des Pins als Digital-Ausgang
	GPIO_InitStructure.GPIO_Pin = WS2812_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(WS2812_PORT, &GPIO_InitStructure);

	// Low-Pegel ausgeben
	WS2812_PORT->BSRRH = WS2812_PIN;

	// Alternative-Funktion mit dem IO-Pin verbinden
	GPIO_PinAFConfig(WS2812_PORT, WS2812_SOURCE, WS2812_TIM_AF);
}


void p_ws2812_init_timer()
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	// Clock enable (TIM)
	RCC_APB1PeriphClockCmd(WS2812_TIM_CLOCK, ENABLE);

	// Clock Enable (DMA)
	RCC_AHB1PeriphClockCmd(WS2812_DMA_CLOCK, ENABLE);

	// Timer init
	TIM_TimeBaseStructure.TIM_Period = WS2812_TIM_PERIODE;
	TIM_TimeBaseStructure.TIM_Prescaler = WS2812_TIM_PRESCALE;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(WS2812_TIM, &TIM_TimeBaseStructure);

	// deinit
	TIM_OCStructInit(&TIM_OCInitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;


	TIM_OC1Init(WS2812_TIM, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(WS2812_TIM, TIM_OCPreload_Enable);

	// Timer enable
	TIM_ARRPreloadConfig(WS2812_TIM, ENABLE);
}

void p_ws2812_clear_buf()
{
	uint32_t n;

	// init vom Timer Array
	for(n = 0; n < WS2812_TIMER_BUF_LEN; n++)
	{
		WS2812_TIMER_BUF[n] = 0; // 0 => kein Signal
	}
}

void p_ws2812_init_NVIC()
{
	NVIC_InitTypeDef NVIC_InitStructure;

	TIM_DMACmd(WS2812_TIM, WS2812_TIM_DMA_TRG, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void p_ws2812_init_DMA()
{
	DMA_InitTypeDef DMA_InitStructure;

	DMA_Cmd(WS2812_DMA_STREAM, DISABLE);
	DMA_DeInit(WS2812_DMA_STREAM);
	DMA_InitStructure.DMA_Channel = WS2812_DMA_CHANNEL;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIM_CCR_REG;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = ((WS2812_CHAIN_LEN + WS2812_PAUSE_ANZ) * WS2812_BIT_PER_LED);
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 16bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(WS2812_DMA_STREAM, &DMA_InitStructure);

}

// refresh der WS2812 Kette
void ws2812_refresh()
{
	// warte bis DMA-Transfer fertig
	while(ws2812_dma_status!=0);

	// Timer Werte berrechnen (normal mode)
	p_ws2812_calc_timer_buf();

	// DMA starten
	p_ws2812_DMA_start();
}

void p_ws2812_DMA_start()
{
	// status auf "busy" setzen
	ws2812_dma_status=1;

	// enable vom Transfer-Complete Interrupt
	DMA_ITConfig(WS2812_DMA_STREAM, DMA_IT_TC, ENABLE);
	// DMA enable
	DMA_Cmd(WS2812_DMA_STREAM, ENABLE);

	// Timer enable
	TIM_Cmd(WS2812_TIM, ENABLE);

}

// errechnet aus den RGB-Farbwerten der aktiven LEDs
// die notwendigen PWM-Werte fuer die Datenleitung
void p_ws2812_calc_timer_buf()
{
	uint32_t n;
	uint32_t pos;
	rgb24_t led;

	pos=0;
	// timingzeiten fuer alle LEDs setzen
	for(n = 0; n < WS2812_CHAIN_LEN; n++)
	{
		led = WS2812_LED_BUF[n];

		// Col:Green , Bit:7..0
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.g & 0x80) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.g & 0x40) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.g & 0x20) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.g & 0x10) !=0 ) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
		if((led.g & 0x08) !=0 ) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
		if((led.g & 0x04) !=0 ) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
		if((led.g & 0x02) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
		if((led.g & 0x01) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;

		// Col:Red , Bit:7..0
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.r & 0x80) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.r & 0x40) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.r & 0x20) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.r & 0x10) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.r & 0x08) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.r & 0x04) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.r & 0x02) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.r & 0x01) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;

		// Col:Blue , Bit:7..0
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.b & 0x80) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.b & 0x40) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.b & 0x20) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.b & 0x10) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.b & 0x08) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.b & 0x04) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.b & 0x02) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
		WS2812_TIMER_BUF[pos] = WS2812_LO_TIME;
		if((led.b & 0x01) != 0) WS2812_TIMER_BUF[pos] = WS2812_HI_TIME;
		pos++;
	}


	// nach den Farbinformationen eine Pausenzeit anhaengen (2*30ms)
	for(n = 0; n < WS2812_PAUSE_ANZ * WS2812_BIT_PER_LED; n++)
	{
		WS2812_TIMER_BUF[pos] = 0;  // 0 => fuer Pausenzeit
		pos++;
	}
}

// ISR vom DMA
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
void WS2812_DMA_ISR()
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_STREAM, WS2812_DMA_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_STREAM, WS2812_DMA_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIM, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_STREAM, DISABLE);

    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}
