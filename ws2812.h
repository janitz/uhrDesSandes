#ifndef WS2812
#define WS2812

//from http://mikrocontroller.bplaced.net/wordpress/?page_id=4204

#include <stdint.h>
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_dma.h>
#include <misc.h>
#include <stm32f4xx_syscfg.h>

typedef struct {
  uint8_t r;  // 0...255
  uint8_t g;  // 0...255
  uint8_t b;  // 0...255
}rgb24_t;

#define RGB_BLACK ((rgb24_t) {0x00, 0x00, 0x00})

#define WS2812_CHAIN_LEN 129 //(2*(8*8)) + 1

#define WS2812_BIT_PER_LED    24  // 3*8bit per led
#define WS2812_PAUSE_ANZ       2  // 2 leds pause (2*30us)
#define WS2812_TIMER_BUF_LEN    ((WS2812_CHAIN_LEN + WS2812_PAUSE_ANZ) * WS2812_BIT_PER_LED)

#define WS2812_CLOCK    RCC_AHB1Periph_GPIOC
#define WS2812_PORT     GPIOC
#define WS2812_PIN      GPIO_Pin_6
#define WS2812_SOURCE   GPIO_PinSource6

#define  WS2812_DMA_IRQn      DMA1_Stream4_IRQn
#define  WS2812_DMA_ISR       DMA1_Stream4_IRQHandler
#define  WS2812_DMA_IRQ_FLAG  DMA_IT_TCIF4

#define  WS2812_TIM_CLOCK     RCC_APB1Periph_TIM3
#define  WS2812_TIM           TIM3
#define  WS2812_TIM_AF        GPIO_AF_TIM3
#define  WS2812_TIM_CH        1
#define  WS2812_TIM_CCR_REG   TIM3->CCR1
#define  WS2812_TIM_DMA_TRG   TIM_DMA_CC1

#define  WS2812_DMA_CLOCK     RCC_AHB1Periph_DMA1
#define  WS2812_DMA_STREAM    DMA1_Stream4
#define  WS2812_DMA_CHANNEL   DMA_Channel_5

#define  WS2812_TIM_PRESCALE    0  // F_T3  = 84 MHz (11.9ns)
#define  WS2812_TIM_PERIODE   104  // F_PWM = 80 kHz (1.25us)

#define  WS2812_LO_TIME        29  // 29 * 11.9ns = 0.34us
#define  WS2812_HI_TIME        76  // 76 * 11.9ns = 0.90us


//public variables
rgb24_t WS2812_LED_BUF[WS2812_CHAIN_LEN];

//public functions
void ws2812_init();
void ws2812_refresh();

#endif
