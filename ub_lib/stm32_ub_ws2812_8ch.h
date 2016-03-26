//--------------------------------------------------------------
// File     : stm32_ub_ws2812_8ch.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_WS2812_H
#define __STM32F4_UB_WS2812_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_dma.h"
#include "misc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"


//--------------------------------------------------------------
// max Anzahl LEDs in einer WS2812-Kette
// (ohne Laengenmessung wird diese Laenge benutzt)
//--------------------------------------------------------------
#define  WS2812_LED_MAX_ANZ    130



//--------------------------------------------------------------
// Enable/Disable der einzelnen Ketten
// 1=enable, 0=disable
//--------------------------------------------------------------
#define  WS2812_CH1_ENABLE   1  // [CH1 an PC6]
#define  WS2812_CH2_ENABLE   0  // [CH2 an PB5]
#define  WS2812_CH3_ENABLE   0  // [CH3 an PB0]
#define  WS2812_CH4_ENABLE   0  // [CH4 an PB1]
#define  WS2812_CH5_ENABLE   0  // [CH5 an PE9]
#define  WS2812_CH6_ENABLE   0  // [CH6 an PE11]
#define  WS2812_CH7_ENABLE   0  // [CH7 an PE13]
#define  WS2812_CH8_ENABLE   0  // [CH8 an PE14]



//--------------------------------------------------------------
// Struktur der WS2812-Ketten laengen
// (wird nach PowerOn aktuallisiert)
//--------------------------------------------------------------
typedef struct {
  uint32_t ch1;      // anzahl der LEDs an CH1
  uint32_t ch2;      // anzahl der LEDs an CH2
  uint32_t ch3;      // anzahl der LEDs an CH3
  uint32_t ch4;      // anzahl der LEDs an CH4
  uint32_t ch5;      // anzahl der LEDs an CH5
  uint32_t ch6;      // anzahl der LEDs an CH6
  uint32_t ch7;      // anzahl der LEDs an CH7
  uint32_t ch8;      // anzahl der LEDs an CH8
}WS2812_CHAIN_LEN_t;
WS2812_CHAIN_LEN_t WS2812_CHAIN_LEN;



//--------------------------------------------------------------
// benutzer Timer fuer das Daten-Signal => TIM3 (fuer CH1 bis CH4)
//--------------------------------------------------------------
#define  WS2812_TIM_CLOCK     RCC_APB1Periph_TIM3
#define  WS2812_TIM           TIM3
#define  WS2812_TIM_AF        GPIO_AF_TIM3
#define  WS2812_TIM_CH1       1 
#define  WS2812_TIM_CCR_REG1  TIM3->CCR1
#define  WS2812_TIM_DMA_TRG1  TIM_DMA_CC1
#define  WS2812_TIM_CH2       2 
#define  WS2812_TIM_CCR_REG2  TIM3->CCR2
#define  WS2812_TIM_DMA_TRG2  TIM_DMA_CC2
#define  WS2812_TIM_CH3       3 
#define  WS2812_TIM_CCR_REG3  TIM3->CCR3
#define  WS2812_TIM_DMA_TRG3  TIM_DMA_CC3
#define  WS2812_TIM_CH4       4 
#define  WS2812_TIM_CCR_REG4  TIM3->CCR4
#define  WS2812_TIM_DMA_TRG4  TIM_DMA_CC4



//--------------------------------------------------------------
// benutzer Timer fuer das Daten-Signal => TIM1 (fuer CH5 bis CH8)
//--------------------------------------------------------------
#define  WS2812_TIMB_CLOCK     RCC_APB2Periph_TIM1
#define  WS2812_TIMB           TIM1
#define  WS2812_TIMB_AF        GPIO_AF_TIM1
#define  WS2812_TIMB_CH1       1 
#define  WS2812_TIMB_CCR_REG1  TIM1->CCR1
#define  WS2812_TIMB_DMA_TRG1  TIM_DMA_CC1
#define  WS2812_TIMB_CH2       2 
#define  WS2812_TIMB_CCR_REG2  TIM1->CCR2
#define  WS2812_TIMB_DMA_TRG2  TIM_DMA_CC2
#define  WS2812_TIMB_CH3       3 
#define  WS2812_TIMB_CCR_REG3  TIM1->CCR3
#define  WS2812_TIMB_DMA_TRG3  TIM_DMA_CC3
#define  WS2812_TIMB_CH4       4 
#define  WS2812_TIMB_CCR_REG4  TIM1->CCR4
#define  WS2812_TIMB_DMA_TRG4  TIM_DMA_CC4



//--------------------------------------------------------------
// GPIO-Pins (CH1...CH4) fuer Data-OUT 
//
// moegliche Pinbelegungen (bei TIM3)
//   CH1 : [PA6, PB4, PC6]
//   CH2 : [PA7, PB5, PC7]
//   CH3 : [PB0, PC8]
//   CH4 : [PB1, PC9] 
//--------------------------------------------------------------
#define  WS2812_CH1_CLOCK    RCC_AHB1Periph_GPIOC
#define  WS2812_CH1_PORT     GPIOC
#define  WS2812_CH1_PIN      GPIO_Pin_6 
#define  WS2812_CH1_SOURCE   GPIO_PinSource6 

#define  WS2812_CH2_CLOCK    RCC_AHB1Periph_GPIOB
#define  WS2812_CH2_PORT     GPIOB
#define  WS2812_CH2_PIN      GPIO_Pin_5 
#define  WS2812_CH2_SOURCE   GPIO_PinSource5

#define  WS2812_CH3_CLOCK    RCC_AHB1Periph_GPIOB
#define  WS2812_CH3_PORT     GPIOB
#define  WS2812_CH3_PIN      GPIO_Pin_0 
#define  WS2812_CH3_SOURCE   GPIO_PinSource0

#define  WS2812_CH4_CLOCK    RCC_AHB1Periph_GPIOB
#define  WS2812_CH4_PORT     GPIOB
#define  WS2812_CH4_PIN      GPIO_Pin_1 
#define  WS2812_CH4_SOURCE   GPIO_PinSource1



//--------------------------------------------------------------
// GPIO-Pins (CH5...CH8) fuer Data-OUT 
//
// moegliche Pinbelegungen (bei TIM1)
//   CH5 : [PA8 , PE9]
//   CH6 : [PA9 , PE11]
//   CH7 : [PA10, PE13]
//   CH8 : [PA11, PE14] 
//--------------------------------------------------------------
#define  WS2812_CH5_CLOCK    RCC_AHB1Periph_GPIOE
#define  WS2812_CH5_PORT     GPIOE
#define  WS2812_CH5_PIN      GPIO_Pin_9 
#define  WS2812_CH5_SOURCE   GPIO_PinSource9 

#define  WS2812_CH6_CLOCK    RCC_AHB1Periph_GPIOE
#define  WS2812_CH6_PORT     GPIOE
#define  WS2812_CH6_PIN      GPIO_Pin_11 
#define  WS2812_CH6_SOURCE   GPIO_PinSource11

#define  WS2812_CH7_CLOCK    RCC_AHB1Periph_GPIOE
#define  WS2812_CH7_PORT     GPIOE
#define  WS2812_CH7_PIN      GPIO_Pin_13 
#define  WS2812_CH7_SOURCE   GPIO_PinSource13

#define  WS2812_CH8_CLOCK    RCC_AHB1Periph_GPIOE
#define  WS2812_CH8_PORT     GPIOE
#define  WS2812_CH8_PIN      GPIO_Pin_14 
#define  WS2812_CH8_SOURCE   GPIO_PinSource14


//--------------------------------------------------------------
// GPIO-Pins fuer EXTI (EXTI0)
// (ein PIN gemeinsam fuer alle LED-Ketten)
// 
// moegliche Pins : [PA0, PB0, PC0, PD0, PE0, PF0, PG0, PH0, PI0]
//--------------------------------------------------------------
#define  WS2812_EXTI_CLOCK   RCC_AHB1Periph_GPIOD
#define  WS2812_EXTI_PORT    GPIOD
#define  WS2812_EXTI_PIN     GPIO_Pin_0
#define  WS2812_EXTI_SOURCE  EXTI_PortSourceGPIOD



//--------------------------------------------------------------
// benutzer DMA (fuer CH1 bis CH4)
//   => TIM3-CC1 => DMA1, Channel5, Stream4
//   => TIM3-CC2 => DMA1, Channel5, Stream5
//   => TIM3-CC3 => DMA1, Channel5, Stream7
//   => TIM3-CC4 => DMA1, Channel5, Stream2
// (siehe Seite 216+217 vom Referenz Manual)
//--------------------------------------------------------------
#define  WS2812_DMA_CLOCK         RCC_AHB1Periph_DMA1

#define  WS2812_DMA_CH1_STREAM    DMA1_Stream4
#define  WS2812_DMA_CH1_CHANNEL   DMA_Channel_5

#define  WS2812_DMA_CH2_STREAM    DMA1_Stream5
#define  WS2812_DMA_CH2_CHANNEL   DMA_Channel_5

#define  WS2812_DMA_CH3_STREAM    DMA1_Stream7
#define  WS2812_DMA_CH3_CHANNEL   DMA_Channel_5

#define  WS2812_DMA_CH4_STREAM    DMA1_Stream2
#define  WS2812_DMA_CH4_CHANNEL   DMA_Channel_5



//--------------------------------------------------------------
// benutzer DMA (fuer CH5 bis CH8)
//   => TIM1-CC1 => DMA2, Channel6, Stream1
//   => TIM1-CC2 => DMA2, Channel6, Stream2
//   => TIM1-CC3 => DMA2, Channel6, Stream6
//   => TIM1-CC4 => DMA2, Channel6, Stream4
// (siehe Seite 216+217 vom Referenz Manual)
//--------------------------------------------------------------
#define  WS2812_DMAB_CLOCK        RCC_AHB1Periph_DMA2

#define  WS2812_DMA_CH5_STREAM    DMA2_Stream1
#define  WS2812_DMA_CH5_CHANNEL   DMA_Channel_6

#define  WS2812_DMA_CH6_STREAM    DMA2_Stream2
#define  WS2812_DMA_CH6_CHANNEL   DMA_Channel_6

#define  WS2812_DMA_CH7_STREAM    DMA2_Stream6
#define  WS2812_DMA_CH7_CHANNEL   DMA_Channel_6

#define  WS2812_DMA_CH8_STREAM    DMA2_Stream4
#define  WS2812_DMA_CH8_CHANNEL   DMA_Channel_6



//--------------------------------------------------------------
// Transfer-Complete Interrupt (fuer CH1 bis CH4)
//   CC1 => DMA1, Stream4
//   CC2 => DMA1, Stream5
//   CC3 => DMA1, Stream7
//   CC4 => DMA1, Stream2
//--------------------------------------------------------------
#define  WS2812_DMA_CH1_IRQn      DMA1_Stream4_IRQn
#define  WS2812_DMA_CH1_ISR       DMA1_Stream4_IRQHandler
#define  WS2812_DMA_CH1_IRQ_FLAG  DMA_IT_TCIF4

#define  WS2812_DMA_CH2_IRQn      DMA1_Stream5_IRQn
#define  WS2812_DMA_CH2_ISR       DMA1_Stream5_IRQHandler
#define  WS2812_DMA_CH2_IRQ_FLAG  DMA_IT_TCIF5

#define  WS2812_DMA_CH3_IRQn      DMA1_Stream7_IRQn
#define  WS2812_DMA_CH3_ISR       DMA1_Stream7_IRQHandler
#define  WS2812_DMA_CH3_IRQ_FLAG  DMA_IT_TCIF7

#define  WS2812_DMA_CH4_IRQn      DMA1_Stream2_IRQn
#define  WS2812_DMA_CH4_ISR       DMA1_Stream2_IRQHandler
#define  WS2812_DMA_CH4_IRQ_FLAG  DMA_IT_TCIF2



//--------------------------------------------------------------
// Transfer-Complete Interrupt (fuer CH5 bis CH8)
//   CC1 => DMA2, Stream1
//   CC2 => DMA2, Stream2
//   CC3 => DMA2, Stream6
//   CC4 => DMA2, Stream4
//--------------------------------------------------------------
#define  WS2812_DMA_CH5_IRQn      DMA2_Stream1_IRQn
#define  WS2812_DMA_CH5_ISR       DMA2_Stream1_IRQHandler
#define  WS2812_DMA_CH5_IRQ_FLAG  DMA_IT_TCIF1

#define  WS2812_DMA_CH6_IRQn      DMA2_Stream2_IRQn
#define  WS2812_DMA_CH6_ISR       DMA2_Stream2_IRQHandler
#define  WS2812_DMA_CH6_IRQ_FLAG  DMA_IT_TCIF2

#define  WS2812_DMA_CH7_IRQn      DMA2_Stream6_IRQn
#define  WS2812_DMA_CH7_ISR       DMA2_Stream6_IRQHandler
#define  WS2812_DMA_CH7_IRQ_FLAG  DMA_IT_TCIF6

#define  WS2812_DMA_CH8_IRQn      DMA2_Stream4_IRQn
#define  WS2812_DMA_CH8_ISR       DMA2_Stream4_IRQHandler
#define  WS2812_DMA_CH8_IRQ_FLAG  DMA_IT_TCIF4



//--------------------------------------------------------------
// RGB LED Farbdefinition (3 x 8bit)
//--------------------------------------------------------------
typedef struct {
  uint8_t red;    // 0...255 (als PWM-Wert)
  uint8_t green;  // 0...255 (als PWM-Wert)
  uint8_t blue;   // 0...255 (als PWM-Wert)
}WS2812_RGB_t;


//--------------------------------------------------------------
// HSV LED Farbdefinition
//--------------------------------------------------------------
typedef struct {
  uint16_t h;     // 0...359 (in Grad, 0=R, 120=G, 240=B)
  uint8_t s;      // 0...100 (in Prozent)
  uint8_t v;      // 0...100 (in Prozent)
}WS2812_HSV_t;



//--------------------------------------------------------------
// Globale Buffer fuer die Farben (als RGB-Wert)
//--------------------------------------------------------------
#if WS2812_CH1_ENABLE>0
  WS2812_RGB_t WS2812_LED_BUF_CH1[WS2812_LED_MAX_ANZ];
#endif
#if WS2812_CH2_ENABLE>0
  WS2812_RGB_t WS2812_LED_BUF_CH2[WS2812_LED_MAX_ANZ];
#endif
#if WS2812_CH3_ENABLE>0
  WS2812_RGB_t WS2812_LED_BUF_CH3[WS2812_LED_MAX_ANZ];
#endif
#if WS2812_CH4_ENABLE>0
  WS2812_RGB_t WS2812_LED_BUF_CH4[WS2812_LED_MAX_ANZ];
#endif
#if WS2812_CH5_ENABLE>0
  WS2812_RGB_t WS2812_LED_BUF_CH5[WS2812_LED_MAX_ANZ];
#endif
#if WS2812_CH6_ENABLE>0
  WS2812_RGB_t WS2812_LED_BUF_CH6[WS2812_LED_MAX_ANZ];
#endif
#if WS2812_CH7_ENABLE>0
  WS2812_RGB_t WS2812_LED_BUF_CH7[WS2812_LED_MAX_ANZ];
#endif
#if WS2812_CH8_ENABLE>0
  WS2812_RGB_t WS2812_LED_BUF_CH8[WS2812_LED_MAX_ANZ];
#endif


//--------------------------------------------------------------
// standard Farben (R,G,B)
//--------------------------------------------------------------
#define  WS2812_RGB_COL_OFF      (WS2812_RGB_t) {0x00,0x00,0x00}

#define  WS2812_RGB_COL_BLUE     (WS2812_RGB_t) {0x00,0x00,0xFF}
#define  WS2812_RGB_COL_GREEN    (WS2812_RGB_t) {0x00,0xFF,0x00}
#define  WS2812_RGB_COL_RED      (WS2812_RGB_t) {0xFF,0x00,0x00}
#define  WS2812_RGB_COL_WHITE    (WS2812_RGB_t) {0xFF,0xFF,0xFF}

#define  WS2812_RGB_COL_CYAN     (WS2812_RGB_t) {0x00,0xFF,0xFF}
#define  WS2812_RGB_COL_MAGENTA  (WS2812_RGB_t) {0xFF,0x00,0xFF}
#define  WS2812_RGB_COL_YELLOW   (WS2812_RGB_t) {0xFF,0xFF,0x00}


//--------------------------------------------------------------
// standard Farben (H,S,V)
//--------------------------------------------------------------
#define  WS2812_HSV_COL_OFF      (WS2812_HSV_t) {0,  0,  0}

#define  WS2812_HSV_COL_BLUE     (WS2812_HSV_t) {240,100,100}
#define  WS2812_HSV_COL_GREEN    (WS2812_HSV_t) {120,100,100}
#define  WS2812_HSV_COL_RED      (WS2812_HSV_t) {0,  100,100}

#define  WS2812_HSV_COL_CYAN     (WS2812_HSV_t) {180,100,100}
#define  WS2812_HSV_COL_MAGENTA  (WS2812_HSV_t) {300,100,100}
#define  WS2812_HSV_COL_YELLOW   (WS2812_HSV_t) {60, 100,100}


//--------------------------------------------------------------
// WS2812 Timing : (1.25us = 800 kHz)
//   logische-0 => HI:0.35us , LO:0.90us
//   logische-1 =  HI:0.90us , LO:0.35us
//
// WS23812 Bit-Format : (8G8R8B)
//   24bit pro LED  (30us pro LED)
//    8bit pro Farbe (MSB first)
//    Farbreihenfolge : GRB
//      
//   nach jedem Frame von n-LEDs kommt eine Pause von >= 50us
//
// Grundfrequenz (TIM3) = 2*APB1 (APB1=42MHz) => TIM_CLK=84MHz
// periode   : 0 bis 0xFFFF
// prescale  : 0 bis 0xFFFF
//
// PWM-Frq = TIM_CLK/(periode+1)/(vorteiler+1)
//-------------------------------------------------------------- 
#define  WS2812_TIM_PRESCALE    0  // F_T3  = 84 MHz (11.9ns)
#define  WS2812_TIM_PERIODE   104  // F_PWM = 80 kHz (1.25us)


#define  WS2812_LO_TIME        29  // 29 * 11.9ns = 0.34us
#define  WS2812_HI_TIME        76  // 76 * 11.9ns = 0.90us




//--------------------------------------------------------------
// Grundfrequenz (TIM1) = 2*APB2 (APB2=84MHz) => TIM_CLK=168MHz
// periode   : 0 bis 0xFFFF
// prescale  : 0 bis 0xFFFF
//
// PWM-Frq = TIM_CLK/(periode+1)/(vorteiler+1)
//-------------------------------------------------------------- 
#define  WS2812_TIMB_PRESCALE    1  // F_T1  = 84 MHz (11.9ns)
#define  WS2812_TIMB_PERIODE   104  // F_PWM = 80 kHz (1.25us)

// LO und HI-Time wie bei Timer-3





//--------------------------------------------------------------
// defines vom WS2812 (nicht abaendern)
//--------------------------------------------------------------
#define  WS2812_BIT_PER_LED    24  // 3*8bit pro LED
#define  WS2812_PAUSE_ANZ       2  // fuer Pause (2*30us)


#define  WS2812_TIMER_BUF_LEN    (WS2812_LED_MAX_ANZ+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED




//--------------------------------------------------------------
// Struktur von einem Font (max 16 Pixel Breite)
//-------------------------------------------------------------- 
typedef struct UB_Font_t {
  const uint16_t *table; // Tabelle mit den Daten
  uint16_t width;        // Breite eines Zeichens (in Pixel)
  uint16_t height;       // Hoehe eines Zeichens  (in Pixel)
}UB_Font; 


//--------------------------------------------------------------
// Aktivierung vom Font
// fuer alle benutzten Fonts muss das entsprechende C-File
// in der CooCox-IDE hinzugefuegt werden
//--------------------------------------------------------------
extern UB_Font Font_6x8;  // Font muss 8Pixel hoch sein !




//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
ErrorStatus UB_WS2812_Init(void);
void UB_WS2812_RGB_2_HSV(WS2812_HSV_t hsv_col, WS2812_RGB_t *rgb_col);
void UB_WS2812_Pixel_RGB(uint32_t x, uint8_t y, WS2812_RGB_t rgb_col);
void UB_WS2812_All_RGB(uint8_t ch_maske, WS2812_RGB_t rgb_col);
void UB_WS2812_Shift_Left(uint8_t ch_maske);
void UB_WS2812_Shift_Right(uint8_t ch_maske);
void UB_WS2812_Rotate_Left(uint8_t ch_maske);
void UB_WS2812_Rotate_Right(uint8_t ch_maske);
void UB_WS2812_Char_RGB(uint32_t x, uint8_t ascii, WS2812_RGB_t vg, WS2812_RGB_t bg);
void UB_WS2812_String_RGB(uint32_t x, char *ptr, WS2812_RGB_t vg, WS2812_RGB_t bg);
void UB_WS2812_RefreshAll(void);
ErrorStatus UB_WS2812_Check_Len(void);




//--------------------------------------------------------------
#endif // __STM32F4_UB_WS2812_H
