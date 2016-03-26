//--------------------------------------------------------------
// File     : stm32_ub_ws2812_8ch.c
// Datum    : 04.10.2014
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, TIM, DMA, MISC, EXTI, SYSCFG
// Funktion : RGB-LED mit WS2812-Chip (LED Vcc = 3,3V !!)
//            (einzelne LED oder bis zu 8 Ketten von n-LEDs)
//
// Hinweis  : es koennen bis zu 8 LED-Ketten betrieben werden
//            die benutzten GPIO-Pins muessen im H-File eingestellt werden
//            die max Anzahl der LEDs muss auch im H-File eingestellt werden
//
// CH1 = PC6
// CH2 = PB5
// CH3 = PB0
// CH4 = PB1
// CH5 = PE9
// CH6 = PE11
// CH7 = PE13
// CH8 = PE14
//
// EXTI = PD0 (zum messen der Anzahl der LEDs aller WS2812-Ketten)
//
// Hinweis : der Ausgang der letzten LED von allen Ketten muss
//           ueber je eine Diode (BAT46) zum EXTI-Pin geleitet werden
//            WS2812_OUT -> Anode -> Kathode -> EXTI
//  (bei Fehler in Laengenmessung wird "WS2812_LED_MAX_ANZ" benutzt)
//
// Rechnungen   : LEDs pro Kanal = n  ,  Anzahl Kanaele = c
// RAM [Bytes]  : (3*n*c)+(2*((n+2)*24))
// Refresh [us] : c*((n*30)+60)
// Beispiel     : 100 Leds pro Kanal, 8 Kanaele = 7kByte, 24ms
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_ws2812_8ch.h"


//--------------------------------------------------------------
// Globale interne Variabeln
//--------------------------------------------------------------
uint32_t ws2812_dma_status;
uint16_t WS2812_TIMER_BUF[WS2812_TIMER_BUF_LEN];
uint8_t  ws2812_aktiv_channel;
WS2812_RGB_t *ws2812_ptr;
uint32_t ws2812_maxanz;
uint32_t ws2812_len;
uint8_t  ws2812_ch_changed;


//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void p_WS2812_setChannel(uint8_t ch);
void p_WS2812_refresh(void);
void p_WS2812_Pixel_RGB(uint32_t x, uint8_t ch, WS2812_RGB_t rgb_col);
void p_WS2812_All_RGB(uint8_t ch, WS2812_RGB_t rgb_col);
void p_WS2812_Shift_Left(uint8_t ch);
void p_WS2812_Shift_Right(uint8_t ch);
void p_WS2812_Rotate_Left(uint8_t ch);
void p_WS2812_Rotate_Right(uint8_t ch);
void p_WS2812_clearDMA(void);
void p_WS2812_clearAll(void);
void p_WS2812_clearBuff(void);
void p_WS2812_calcTimerBuf(uint8_t mode);
void p_WS2812_InitIO(void);
void p_WS2812_InitTIM(void);
void p_WS2812_InitDMA(void);
void p_WS2812_InitNVIC(void);
void p_WS2812_InitEXTI(void);
void p_WS2812_DMA_Start(void);



//--------------------------------------------------------------
// init aller WS2812-Ketten
// (alle LEDs ausschalten und messen der Laenge aller Ketten)
// Return_wert :
//  -> ERROR   , wenn laenge einer Kette nicht ok
//  -> SUCCESS , wenn laenge aller Ketten ok
// Laenge der Ketten steht in :
//    "WS2812_CHAIN_LEN.ch1" bis "WS2812_CHAIN_LEN.ch8"
//--------------------------------------------------------------
ErrorStatus UB_WS2812_Init(void)
{
  ErrorStatus ret_wert=SUCCESS;
  uint32_t n;
  uint8_t multi_ch=0;

  // init aller Variabeln
  ws2812_dma_status=0;
  ws2812_aktiv_channel=0;
  ws2812_maxanz=0;
  ws2812_len=0;
  ws2812_ch_changed=0;

  WS2812_CHAIN_LEN.ch1=WS2812_LED_MAX_ANZ;
  WS2812_CHAIN_LEN.ch2=WS2812_LED_MAX_ANZ;
  WS2812_CHAIN_LEN.ch3=WS2812_LED_MAX_ANZ;
  WS2812_CHAIN_LEN.ch4=WS2812_LED_MAX_ANZ;
  WS2812_CHAIN_LEN.ch5=WS2812_LED_MAX_ANZ;
  WS2812_CHAIN_LEN.ch6=WS2812_LED_MAX_ANZ;
  WS2812_CHAIN_LEN.ch7=WS2812_LED_MAX_ANZ;
  WS2812_CHAIN_LEN.ch8=WS2812_LED_MAX_ANZ;

  // Buffer loeschen
  p_WS2812_clearBuff();

  // init der LED Arrays aller Ketten
  multi_ch=0;
  #if WS2812_CH8_ENABLE>0
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH8[n]=WS2812_RGB_COL_OFF;
    }
    multi_ch|=0x80;
  #endif
  #if WS2812_CH7_ENABLE>0
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH7[n]=WS2812_RGB_COL_OFF;
    }
    multi_ch|=0x40;
  #endif
  #if WS2812_CH6_ENABLE>0
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH6[n]=WS2812_RGB_COL_OFF;
    }
    multi_ch|=0x20;
  #endif
  #if WS2812_CH5_ENABLE>0
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH5[n]=WS2812_RGB_COL_OFF;
    }
    multi_ch|=0x10;
  #endif
  #if WS2812_CH4_ENABLE>0
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH4[n]=WS2812_RGB_COL_OFF;
    }
    multi_ch|=0x08;
  #endif
  #if WS2812_CH3_ENABLE>0
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH3[n]=WS2812_RGB_COL_OFF;
    }
    multi_ch|=0x04;
  #endif
  #if WS2812_CH2_ENABLE>0
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH2[n]=WS2812_RGB_COL_OFF;
    }
    multi_ch|=0x02;
  #endif
  #if WS2812_CH1_ENABLE>0
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH1[n]=WS2812_RGB_COL_OFF;
    }
    multi_ch|=0x01;
  #endif

  if(multi_ch==0) return(ERROR);

  // init vom GPIO
  p_WS2812_InitIO();
  // init vom Timer
  p_WS2812_InitTIM();
  // init vom EXTI
  p_WS2812_InitEXTI();
  // init vom NVIC
  p_WS2812_InitNVIC();
  // init vom DMA
  p_WS2812_InitDMA();

  // einmal DMA starten (ohne Signal)
  p_WS2812_clearDMA();

  // laengen Messung starten
  //ret_wert=UB_WS2812_Check_Len();

  // alle WS2812-Ketten loeschen
  p_WS2812_clearAll();

  return(ret_wert);
}


//--------------------------------------------------------------
// wandelt einen HSV-Farbwert in einen RGB-Farbwert um
// (Funktion von UlrichRadig.de)
//--------------------------------------------------------------
void UB_WS2812_RGB_2_HSV(WS2812_HSV_t hsv_col, WS2812_RGB_t *rgb_col)
{
  uint8_t diff;

  // Grenzwerte
  if(hsv_col.h>359) hsv_col.h=359;
  if(hsv_col.s>100) hsv_col.s=100;
  if(hsv_col.v>100) hsv_col.v=100;

  if(hsv_col.h < 61) {
    rgb_col->red = 255;
    rgb_col->green = (425 * hsv_col.h) / 100;
    rgb_col->blue = 0;
  }else if(hsv_col.h < 121){
    rgb_col->red = 255 - ((425 * (hsv_col.h-60))/100);
    rgb_col->green = 255;
    rgb_col->blue = 0;
  }else if(hsv_col.h < 181){
    rgb_col->red = 0;
    rgb_col->green = 255;
    rgb_col->blue = (425 * (hsv_col.h-120))/100;
  }else if(hsv_col.h < 241){
    rgb_col->red = 0;
    rgb_col->green = 255 - ((425 * (hsv_col.h-180))/100);
    rgb_col->blue = 255;
  }else if(hsv_col.h < 301){
    rgb_col->red = (425 * (hsv_col.h-240))/100;
    rgb_col->green = 0;
    rgb_col->blue = 255;
  }else {
    rgb_col->red = 255;
    rgb_col->green = 0;
    rgb_col->blue = 255 - ((425 * (hsv_col.h-300))/100);
  }

  hsv_col.s = 100 - hsv_col.s;
  diff = ((255 - rgb_col->red) * hsv_col.s)/100;
  rgb_col->red = rgb_col->red + diff;
  diff = ((255 - rgb_col->green) * hsv_col.s)/100;
  rgb_col->green = rgb_col->green + diff;
  diff = ((255 - rgb_col->blue) * hsv_col.s)/100;
  rgb_col->blue = rgb_col->blue + diff;

  rgb_col->red = (rgb_col->red * hsv_col.v)/100;
  rgb_col->green = (rgb_col->green * hsv_col.v)/100;
  rgb_col->blue = (rgb_col->blue * hsv_col.v)/100;
}


//--------------------------------------------------------------
// eine einzelbe LED auf eine Farbe setzen (RGB)
// x       : [0...WS2812_CHAIN_LEN.chx-1]
// y       : [0...7] 0=CH1...7=CH8
// rgb_col : Farbwert (in RGB)
//--------------------------------------------------------------
void UB_WS2812_Pixel_RGB(uint32_t x, uint8_t y, WS2812_RGB_t rgb_col)
{
  p_WS2812_Pixel_RGB(x,y+1,rgb_col);
}


//--------------------------------------------------------------
// alle LEDs der Kette(n) (je nach Maske) auf eine Farbe setzen (RGB)
// ch_maske : Maske für die LED-Kette [0...255] Bit0=CH1...Bit7=CH8
// rgb_col  : Farbwert (in RGB)
//--------------------------------------------------------------
void UB_WS2812_All_RGB(uint8_t ch_maske, WS2812_RGB_t rgb_col)
{
  // je nach Maske die Ketten waehlen
  if((ch_maske&0x01)!=0) p_WS2812_All_RGB(1,rgb_col);
  if((ch_maske&0x02)!=0) p_WS2812_All_RGB(2,rgb_col);
  if((ch_maske&0x04)!=0) p_WS2812_All_RGB(3,rgb_col);
  if((ch_maske&0x08)!=0) p_WS2812_All_RGB(4,rgb_col);
  if((ch_maske&0x10)!=0) p_WS2812_All_RGB(5,rgb_col);
  if((ch_maske&0x20)!=0) p_WS2812_All_RGB(6,rgb_col);
  if((ch_maske&0x40)!=0) p_WS2812_All_RGB(7,rgb_col);
  if((ch_maske&0x80)!=0) p_WS2812_All_RGB(8,rgb_col);
}


//--------------------------------------------------------------
// alle LEDs der Kette(n) (je nach Maske) eine position nach links schieben
// letzte LED wird ausgeschaltet
// ch_maske : Maske für die LED-Kette [0...255] Bit0=CH1...Bit7=CH8
//--------------------------------------------------------------
void UB_WS2812_Shift_Left(uint8_t ch_maske)
{
  // je nach Maske die Ketten waehlen
  if((ch_maske&0x01)!=0) p_WS2812_Shift_Left(1);
  if((ch_maske&0x02)!=0) p_WS2812_Shift_Left(2);
  if((ch_maske&0x04)!=0) p_WS2812_Shift_Left(3);
  if((ch_maske&0x08)!=0) p_WS2812_Shift_Left(4);
  if((ch_maske&0x10)!=0) p_WS2812_Shift_Left(5);
  if((ch_maske&0x20)!=0) p_WS2812_Shift_Left(6);
  if((ch_maske&0x40)!=0) p_WS2812_Shift_Left(7);
  if((ch_maske&0x80)!=0) p_WS2812_Shift_Left(8);
}


//--------------------------------------------------------------
// alle LEDs der Kette(n) (je nach Maske) eine position nach rechts schieben
// erste LED wird ausgeschaltet
// ch_maske : Maske für die LED-Kette [0...255] Bit0=CH1...Bit7=CH8
//--------------------------------------------------------------
void UB_WS2812_Shift_Right(uint8_t ch_maske)
{
  // je nach Maske die Ketten waehlen
  if((ch_maske&0x01)!=0) p_WS2812_Shift_Right(1);
  if((ch_maske&0x02)!=0) p_WS2812_Shift_Right(2);
  if((ch_maske&0x04)!=0) p_WS2812_Shift_Right(3);
  if((ch_maske&0x08)!=0) p_WS2812_Shift_Right(4);
  if((ch_maske&0x10)!=0) p_WS2812_Shift_Right(5);
  if((ch_maske&0x20)!=0) p_WS2812_Shift_Right(6);
  if((ch_maske&0x40)!=0) p_WS2812_Shift_Right(7);
  if((ch_maske&0x80)!=0) p_WS2812_Shift_Right(8);
}


//--------------------------------------------------------------
// alle LEDs der Kette(n) (je nach Maske) eine position nach links rotieren
// letzte LED bekommt den Farbwert der ersten LED
// ch_maske : Maske für die LED-Kette [0...255] Bit0=CH1...Bit7=CH8
//--------------------------------------------------------------
void UB_WS2812_Rotate_Left(uint8_t ch_maske)
{
  // je nach Maske die Ketten waehlen
  if((ch_maske&0x01)!=0) p_WS2812_Rotate_Left(1);
  if((ch_maske&0x02)!=0) p_WS2812_Rotate_Left(2);
  if((ch_maske&0x04)!=0) p_WS2812_Rotate_Left(3);
  if((ch_maske&0x08)!=0) p_WS2812_Rotate_Left(4);
  if((ch_maske&0x10)!=0) p_WS2812_Rotate_Left(5);
  if((ch_maske&0x20)!=0) p_WS2812_Rotate_Left(6);
  if((ch_maske&0x40)!=0) p_WS2812_Rotate_Left(7);
  if((ch_maske&0x80)!=0) p_WS2812_Rotate_Left(8);
}


//--------------------------------------------------------------
// alle LEDs der Kette(n) (je nach Maske) eine position nach rechts rotieren
// erste LED bekommt den Farbwert der letzten LED
// ch_maske : Maske für die LED-Kette [0...255] Bit0=CH1...Bit7=CH8
//--------------------------------------------------------------
void UB_WS2812_Rotate_Right(uint8_t ch_maske)
{
  // je nach Maske die Ketten waehlen
  if((ch_maske&0x01)!=0) p_WS2812_Rotate_Right(1);
  if((ch_maske&0x02)!=0) p_WS2812_Rotate_Right(2);
  if((ch_maske&0x04)!=0) p_WS2812_Rotate_Right(3);
  if((ch_maske&0x08)!=0) p_WS2812_Rotate_Right(4);
  if((ch_maske&0x10)!=0) p_WS2812_Rotate_Right(5);
  if((ch_maske&0x20)!=0) p_WS2812_Rotate_Right(6);
  if((ch_maske&0x40)!=0) p_WS2812_Rotate_Right(7);
  if((ch_maske&0x80)!=0) p_WS2812_Rotate_Right(8);
}


//--------------------------------------------------------------
// Zeichnet ein Ascii-Zeichen an eine Position
// mit Vorder- und Hintergrundfarbe (Font = 8 Pixel hoehe)
// x     : [0...WS2812_CHAIN_LEN.chx-1]
// ascii : Ascii-Zeichen (32 bis 128)
// vg    : Vordergrundfarbe (in RGB)
// bg    : Hintergrundfarbe (in RGB)
//--------------------------------------------------------------
void UB_WS2812_Char_RGB(uint32_t x, uint8_t ascii, WS2812_RGB_t vg, WS2812_RGB_t bg)
{
  uint16_t xn,yn,maske;
  const uint16_t *wert;

  ascii -= 32;
  wert=&Font_6x8.table[ascii * Font_6x8.height];

  // Font muss 8Pixel hoch sein
  if(Font_6x8.height!=8) return;

  for(yn = 0; yn < Font_6x8.height; yn++) {
    maske=0x80;
    for(xn = 0; xn < Font_6x8.width; xn++) {
      if((wert[yn] & maske) == 0x00) {
        // LED in Hintergrundfarbe zeichnen
        UB_WS2812_Pixel_RGB(x+xn,yn,bg);
      }
      else {
        // LED in Vordergrundfarbe zeichnen
        UB_WS2812_Pixel_RGB(x+xn,yn,vg);
      }
      maske=(maske>>1);
    }
  }
}


//--------------------------------------------------------------
// Zeichnet einen String an eine Position
// mit Vorder- und Hintergrundfarbe (Font = 8 Pixel hoehe)
// x   : [0...WS2812_CHAIN_LEN.chx-1]
// ptr : Pointer auf einen String
// vg  : Vordergrundfarbe (in RGB)
// bg  : Hintergrundfarbe (in RGB)
//--------------------------------------------------------------
void UB_WS2812_String_RGB(uint32_t x, char *ptr, WS2812_RGB_t vg, WS2812_RGB_t bg)
{
  uint32_t pos;

  pos=x;
  while (*ptr != 0) {
    UB_WS2812_Char_RGB(pos,*ptr,vg,bg);
    pos+=Font_6x8.width;
    ptr++;
  }
}


//--------------------------------------------------------------
// Refresh aller geänderten WS2812-Kette(n)
// (update aller LEDs)
// Die RGB-Farbwerte der LEDs muss im Array "WS2812_LED_BUF_CHx[n]" stehen
// n = [0...WS2812_CHAIN_LEN.chx-1]
//--------------------------------------------------------------
void UB_WS2812_RefreshAll(void)
{
  if(ws2812_ch_changed==0) return;

  // alle geaenderten Ketten refreshen
  if((ws2812_ch_changed&0x01)!=0) {
    p_WS2812_setChannel(1);
    p_WS2812_refresh();
  }
  if((ws2812_ch_changed&0x02)!=0) {
    p_WS2812_setChannel(2);
    p_WS2812_refresh();
  }
  if((ws2812_ch_changed&0x04)!=0) {
    p_WS2812_setChannel(3);
    p_WS2812_refresh();
  }
  if((ws2812_ch_changed&0x08)!=0) {
    p_WS2812_setChannel(4);
    p_WS2812_refresh();
  }
  if((ws2812_ch_changed&0x10)!=0) {
    p_WS2812_setChannel(5);
    p_WS2812_refresh();
  }
  if((ws2812_ch_changed&0x20)!=0) {
    p_WS2812_setChannel(6);
    p_WS2812_refresh();
  }
  if((ws2812_ch_changed&0x40)!=0) {
    p_WS2812_setChannel(7);
    p_WS2812_refresh();
  }
  if((ws2812_ch_changed&0x80)!=0) {
    p_WS2812_setChannel(8);
    p_WS2812_refresh();
  }

  // Ketten als refreshed markieren
  ws2812_ch_changed=0;
}


//--------------------------------------------------------------
// testet die Anzahl der LEDs in den WS2812-Ketten
// Return_wert :
//  -> ERROR   , wenn laenge einer Kette nicht ok
//  -> SUCCESS , wenn laenge aller Ketten ok
// Laenge der Ketten steht in :
//    "WS2812_CHAIN_LEN.ch1" bis "WS2812_CHAIN_LEN.ch8"
//--------------------------------------------------------------
ErrorStatus UB_WS2812_Check_Len(void)
{
  ErrorStatus ret_wert=SUCCESS;
  uint32_t n;

  // warte bis DMA-Transfer fertig
  while(ws2812_dma_status!=0);

  #if WS2812_CH1_ENABLE>0
    // laenge auf max
    WS2812_CHAIN_LEN.ch1=WS2812_LED_MAX_ANZ;
    p_WS2812_setChannel(1);

    // alle LEDs ausschalten
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH1[n]=WS2812_RGB_COL_OFF;
    }
    // Timer Werte berrechnen (fuer Messung)
    p_WS2812_calcTimerBuf(1);

    // laenge zuruecksetzen
    ws2812_len=0;

    // DMA starten
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // buffer loeschen und DMA nochmal starten
    p_WS2812_clearBuff();
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // gemessene laenge setzen
    WS2812_CHAIN_LEN.ch1=(ws2812_len/WS2812_BIT_PER_LED);
    if(WS2812_CHAIN_LEN.ch1==0) {
      WS2812_CHAIN_LEN.ch1=WS2812_LED_MAX_ANZ;
      ret_wert=ERROR;
    }
  #endif

  #if WS2812_CH2_ENABLE>0
    // laenge auf max
    WS2812_CHAIN_LEN.ch2=WS2812_LED_MAX_ANZ;
    p_WS2812_setChannel(2);

    // alle LEDs ausschalten
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH2[n]=WS2812_RGB_COL_OFF;
    }
    // Timer Werte berrechnen (fuer Messung)
    p_WS2812_calcTimerBuf(1);

    // laenge zuruecksetzen
    ws2812_len=0;

    // DMA starten
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // buffer loeschen und DMA nochmal starten
    p_WS2812_clearBuff();
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // gemessene laenge setzen
    WS2812_CHAIN_LEN.ch2=(ws2812_len/WS2812_BIT_PER_LED);
    if(WS2812_CHAIN_LEN.ch2==0) {
      WS2812_CHAIN_LEN.ch2=WS2812_LED_MAX_ANZ;
      ret_wert=ERROR;
    }
  #endif

  #if WS2812_CH3_ENABLE>0
    // laenge auf max
    WS2812_CHAIN_LEN.ch3=WS2812_LED_MAX_ANZ;
    p_WS2812_setChannel(3);

    // alle LEDs ausschalten
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH3[n]=WS2812_RGB_COL_OFF;
    }
    // Timer Werte berrechnen (fuer Messung)
    p_WS2812_calcTimerBuf(1);

    // laenge zuruecksetzen
    ws2812_len=0;

    // DMA starten
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // buffer loeschen und DMA nochmal starten
    p_WS2812_clearBuff();
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // gemessene laenge setzen
    WS2812_CHAIN_LEN.ch3=(ws2812_len/WS2812_BIT_PER_LED);
    if(WS2812_CHAIN_LEN.ch3==0) {
      WS2812_CHAIN_LEN.ch3=WS2812_LED_MAX_ANZ;
      ret_wert=ERROR;
    }
  #endif

  #if WS2812_CH4_ENABLE>0
    // laenge auf max
    WS2812_CHAIN_LEN.ch4=WS2812_LED_MAX_ANZ;
    p_WS2812_setChannel(4);

    // alle LEDs ausschalten
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH4[n]=WS2812_RGB_COL_OFF;
    }
    // Timer Werte berrechnen (fuer Messung)
    p_WS2812_calcTimerBuf(1);

    // laenge zuruecksetzen
    ws2812_len=0;

    // DMA starten
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // buffer loeschen und DMA nochmal starten
    p_WS2812_clearBuff();
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // gemessene laenge setzen
    WS2812_CHAIN_LEN.ch4=(ws2812_len/WS2812_BIT_PER_LED);
    if(WS2812_CHAIN_LEN.ch4==0) {
      WS2812_CHAIN_LEN.ch4=WS2812_LED_MAX_ANZ;
      ret_wert=ERROR;
    }
  #endif

  #if WS2812_CH5_ENABLE>0
    // laenge auf max
    WS2812_CHAIN_LEN.ch5=WS2812_LED_MAX_ANZ;
    p_WS2812_setChannel(5);

    // alle LEDs ausschalten
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH5[n]=WS2812_RGB_COL_OFF;
    }
    // Timer Werte berrechnen (fuer Messung)
    p_WS2812_calcTimerBuf(1);

    // laenge zuruecksetzen
    ws2812_len=0;

    // DMA starten
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // buffer loeschen und DMA nochmal starten
    p_WS2812_clearBuff();
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // gemessene laenge setzen
    WS2812_CHAIN_LEN.ch5=(ws2812_len/WS2812_BIT_PER_LED);
    if(WS2812_CHAIN_LEN.ch5==0) {
      WS2812_CHAIN_LEN.ch5=WS2812_LED_MAX_ANZ;
      ret_wert=ERROR;
    }
  #endif

  #if WS2812_CH6_ENABLE>0
    // laenge auf max
    WS2812_CHAIN_LEN.ch6=WS2812_LED_MAX_ANZ;
    p_WS2812_setChannel(6);

    // alle LEDs ausschalten
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH6[n]=WS2812_RGB_COL_OFF;
    }
    // Timer Werte berrechnen (fuer Messung)
    p_WS2812_calcTimerBuf(1);

    // laenge zuruecksetzen
    ws2812_len=0;

    // DMA starten
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // buffer loeschen und DMA nochmal starten
    p_WS2812_clearBuff();
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // gemessene laenge setzen
    WS2812_CHAIN_LEN.ch6=(ws2812_len/WS2812_BIT_PER_LED);
    if(WS2812_CHAIN_LEN.ch6==0) {
      WS2812_CHAIN_LEN.ch6=WS2812_LED_MAX_ANZ;
      ret_wert=ERROR;
    }
  #endif

  #if WS2812_CH7_ENABLE>0
    // laenge auf max
    WS2812_CHAIN_LEN.ch7=WS2812_LED_MAX_ANZ;
    p_WS2812_setChannel(7);

    // alle LEDs ausschalten
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH7[n]=WS2812_RGB_COL_OFF;
    }
    // Timer Werte berrechnen (fuer Messung)
    p_WS2812_calcTimerBuf(1);

    // laenge zuruecksetzen
    ws2812_len=0;

    // DMA starten
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // buffer loeschen und DMA nochmal starten
    p_WS2812_clearBuff();
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // gemessene laenge setzen
    WS2812_CHAIN_LEN.ch7=(ws2812_len/WS2812_BIT_PER_LED);
    if(WS2812_CHAIN_LEN.ch7==0) {
      WS2812_CHAIN_LEN.ch7=WS2812_LED_MAX_ANZ;
      ret_wert=ERROR;
    }
  #endif

  #if WS2812_CH8_ENABLE>0
    // laenge auf max
    WS2812_CHAIN_LEN.ch8=WS2812_LED_MAX_ANZ;
    p_WS2812_setChannel(8);

    // alle LEDs ausschalten
    for(n=0;n<WS2812_LED_MAX_ANZ;n++) {
      WS2812_LED_BUF_CH8[n]=WS2812_RGB_COL_OFF;
    }
    // Timer Werte berrechnen (fuer Messung)
    p_WS2812_calcTimerBuf(1);

    // laenge zuruecksetzen
    ws2812_len=0;

    // DMA starten
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // buffer loeschen und DMA nochmal starten
    p_WS2812_clearBuff();
    p_WS2812_DMA_Start();
    while(ws2812_dma_status!=0);

    // gemessene laenge setzen
    WS2812_CHAIN_LEN.ch8=(ws2812_len/WS2812_BIT_PER_LED);
    if(WS2812_CHAIN_LEN.ch8==0) {
      WS2812_CHAIN_LEN.ch8=WS2812_LED_MAX_ANZ;
      ret_wert=ERROR;
    }
  #endif

  // laenge der nichtbenutzten Ketten auf 0 setzen
  if(WS2812_CH1_ENABLE==0) WS2812_CHAIN_LEN.ch1=0;
  if(WS2812_CH2_ENABLE==0) WS2812_CHAIN_LEN.ch2=0;
  if(WS2812_CH3_ENABLE==0) WS2812_CHAIN_LEN.ch3=0;
  if(WS2812_CH4_ENABLE==0) WS2812_CHAIN_LEN.ch4=0;
  if(WS2812_CH5_ENABLE==0) WS2812_CHAIN_LEN.ch5=0;
  if(WS2812_CH6_ENABLE==0) WS2812_CHAIN_LEN.ch6=0;
  if(WS2812_CH7_ENABLE==0) WS2812_CHAIN_LEN.ch7=0;
  if(WS2812_CH8_ENABLE==0) WS2812_CHAIN_LEN.ch8=0;  

  // messung deaktivieren
  ws2812_len=1;

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// aktiviert eine einzelne WS2812 Kette
// ch : [1...8]
//--------------------------------------------------------------
void p_WS2812_setChannel(uint8_t ch)
{
  ws2812_aktiv_channel=0;

  #if WS2812_CH1_ENABLE>0
    if(ch==1) {
      ws2812_aktiv_channel=1;
      ws2812_ptr=&WS2812_LED_BUF_CH1[0];
      ws2812_maxanz=WS2812_CHAIN_LEN.ch1;      
    }
  #endif
  #if WS2812_CH2_ENABLE>0
    if(ch==2) {
      ws2812_aktiv_channel=2;
      ws2812_ptr=&WS2812_LED_BUF_CH2[0];
      ws2812_maxanz=WS2812_CHAIN_LEN.ch2;
    }
  #endif
  #if WS2812_CH3_ENABLE>0
    if(ch==3) {
      ws2812_aktiv_channel=3;
      ws2812_ptr=&WS2812_LED_BUF_CH3[0];
      ws2812_maxanz=WS2812_CHAIN_LEN.ch3;
    }
  #endif
  #if WS2812_CH4_ENABLE>0
    if(ch==4) {
      ws2812_aktiv_channel=4;
      ws2812_ptr=&WS2812_LED_BUF_CH4[0];
      ws2812_maxanz=WS2812_CHAIN_LEN.ch4;
    }
  #endif
  #if WS2812_CH5_ENABLE>0
    if(ch==5) {
      ws2812_aktiv_channel=5;
      ws2812_ptr=&WS2812_LED_BUF_CH5[0];
      ws2812_maxanz=WS2812_CHAIN_LEN.ch5;
    }
  #endif
  #if WS2812_CH6_ENABLE>0
    if(ch==6) {
      ws2812_aktiv_channel=6;
      ws2812_ptr=&WS2812_LED_BUF_CH6[0];
      ws2812_maxanz=WS2812_CHAIN_LEN.ch6;
    }
  #endif
  #if WS2812_CH7_ENABLE>0
    if(ch==7) {
      ws2812_aktiv_channel=7;
      ws2812_ptr=&WS2812_LED_BUF_CH7[0];
      ws2812_maxanz=WS2812_CHAIN_LEN.ch7;
    }
  #endif
  #if WS2812_CH8_ENABLE>0
    if(ch==8) {
      ws2812_aktiv_channel=8;
      ws2812_ptr=&WS2812_LED_BUF_CH8[0];
      ws2812_maxanz=WS2812_CHAIN_LEN.ch8;
    }
  #endif
}


//--------------------------------------------------------------
// interne Funktion
// refresh der aktiven WS2812 Kette
//--------------------------------------------------------------
void p_WS2812_refresh(void)
{
  if(ws2812_aktiv_channel==0) return;

  // warte bis DMA-Transfer fertig
  while(ws2812_dma_status!=0);

  // Timer Werte berrechnen (normal mode)
  p_WS2812_calcTimerBuf(0);

  // DMA starten
  p_WS2812_DMA_Start();  
}


//--------------------------------------------------------------
// interne Funktion
// setzt eine einzelne LED eines Kanals auf eine Farbe
// ch : [1...8]
//--------------------------------------------------------------
void p_WS2812_Pixel_RGB(uint32_t x, uint8_t ch, WS2812_RGB_t rgb_col)
{
  p_WS2812_setChannel(ch);

  if(ws2812_aktiv_channel==0) return;

  if(x<ws2812_maxanz) {
    ws2812_ptr[x]=rgb_col;
    // kette als geaendert markieren
    ws2812_ch_changed|=(1<<(ws2812_aktiv_channel-1));
  }
}


//--------------------------------------------------------------
// interne Funktion
// setzt alle LEDs vom Kanal auf eine Farbe
// ch : [1...8]
//--------------------------------------------------------------
void p_WS2812_All_RGB(uint8_t ch, WS2812_RGB_t rgb_col)
{
  uint32_t n;

  p_WS2812_setChannel(ch);

  if(ws2812_aktiv_channel==0) return;

  for(n=0;n<ws2812_maxanz;n++) {
    ws2812_ptr[n]=rgb_col;
  }
  // kette als geaendert markieren
  ws2812_ch_changed|=(1<<(ws2812_aktiv_channel-1));
}


//--------------------------------------------------------------
// interne Funktion
// schiebe alle LEDs vom Kanal nach links
// ch : [1...8]
//--------------------------------------------------------------
void p_WS2812_Shift_Left(uint8_t ch)
{
  uint32_t n;

  p_WS2812_setChannel(ch);

  if(ws2812_aktiv_channel==0) return;

  if(ws2812_maxanz>1) {
    for(n=1;n<ws2812_maxanz;n++) {
      ws2812_ptr[n-1]=ws2812_ptr[n];
    }
    ws2812_ptr[n-1]=WS2812_RGB_COL_OFF;
 
    // kette als geaendert markieren
    ws2812_ch_changed|=(1<<(ws2812_aktiv_channel-1));
  }
}


//--------------------------------------------------------------
// interne Funktion
// schiebe alle LEDs vom Kanal nach rechts
// ch : [1...8]
//--------------------------------------------------------------
void p_WS2812_Shift_Right(uint8_t ch)
{
  uint32_t n;

  p_WS2812_setChannel(ch);

  if(ws2812_aktiv_channel==0) return;

  if(ws2812_maxanz>1) {
    for(n=ws2812_maxanz-1;n>0;n--) {
      ws2812_ptr[n]=ws2812_ptr[n-1];
    }
    ws2812_ptr[n]=WS2812_RGB_COL_OFF;

    // kette als geaendert markieren
    ws2812_ch_changed|=(1<<(ws2812_aktiv_channel-1));
  }
}


//--------------------------------------------------------------
// interne Funktion
// rotiert alle LEDs vom Kanal nach links
// ch : [1...8]
//--------------------------------------------------------------
void p_WS2812_Rotate_Left(uint8_t ch)
{
  uint32_t n;
  WS2812_RGB_t d;

  p_WS2812_setChannel(ch);

  if(ws2812_aktiv_channel==0) return;

  if(ws2812_maxanz>1) {
    d=ws2812_ptr[0];
    for(n=1;n<ws2812_maxanz;n++) {
      ws2812_ptr[n-1]=ws2812_ptr[n];
    }
    ws2812_ptr[n-1]=d;

    // kette als geaendert markieren
    ws2812_ch_changed|=(1<<(ws2812_aktiv_channel-1));
  }
}


//--------------------------------------------------------------
// interne Funktion
// rotiert alle LEDs vom Kanal nach rechts
// ch : [1...8]
//--------------------------------------------------------------
void p_WS2812_Rotate_Right(uint8_t ch)
{
  uint32_t n;
  WS2812_RGB_t d;

  p_WS2812_setChannel(ch);

  if(ws2812_aktiv_channel==0) return;

  if(ws2812_maxanz>1) {
    d=ws2812_ptr[ws2812_maxanz-1];
    for(n=ws2812_maxanz-1;n>0;n--) {
      ws2812_ptr[n]=ws2812_ptr[n-1];
    }
    ws2812_ptr[n]=d;

    // kette als geaendert markieren
    ws2812_ch_changed|=(1<<(ws2812_aktiv_channel-1));
  }
}


//--------------------------------------------------------------
// interne Funktion
// startet einmal alle DMA Kanaele
//--------------------------------------------------------------
void p_WS2812_clearDMA(void)
{
  // loescht den Buffer
  p_WS2812_clearBuff();

  //-------------------------
  // einmal DMA starten
  // (ohne Signal, Dauer LoPegel)
  //-------------------------
  #if WS2812_CH8_ENABLE>0
    // auf Kanal 8 schalten
    p_WS2812_setChannel(8);
    // warte bis DMA-Transfer fertig
    while(ws2812_dma_status!=0);
    // DMA starten
    p_WS2812_DMA_Start();
  #endif
  #if WS2812_CH7_ENABLE>0
    // auf Kanal 7 schalten
    p_WS2812_setChannel(7);
    // warte bis DMA-Transfer fertig
    while(ws2812_dma_status!=0);
    // DMA starten
    p_WS2812_DMA_Start();
  #endif
  #if WS2812_CH6_ENABLE>0
    // auf Kanal 6 schalten
    p_WS2812_setChannel(6);
    // warte bis DMA-Transfer fertig
    while(ws2812_dma_status!=0);
    // DMA starten
    p_WS2812_DMA_Start();
  #endif
  #if WS2812_CH5_ENABLE>0
    // auf Kanal 5 schalten
    p_WS2812_setChannel(5);
    // warte bis DMA-Transfer fertig
    while(ws2812_dma_status!=0);
    // DMA starten
    p_WS2812_DMA_Start();
  #endif
  #if WS2812_CH4_ENABLE>0
    // auf Kanal 4 schalten
    p_WS2812_setChannel(4);
    // warte bis DMA-Transfer fertig
    while(ws2812_dma_status!=0);
    // DMA starten
    p_WS2812_DMA_Start();
  #endif
  #if WS2812_CH3_ENABLE>0
    // auf Kanal 3 schalten
    p_WS2812_setChannel(3);
    // warte bis DMA-Transfer fertig
    while(ws2812_dma_status!=0);
    // DMA starten
    p_WS2812_DMA_Start();
  #endif
  #if WS2812_CH2_ENABLE>0
    // auf Kanal 2 schalten
    p_WS2812_setChannel(2);
    // warte bis DMA-Transfer fertig
    while(ws2812_dma_status!=0);
    // DMA starten
    p_WS2812_DMA_Start();
  #endif
  #if WS2812_CH1_ENABLE>0
    // auf Kanal 1 schalten
    p_WS2812_setChannel(1);
    // warte bis DMA-Transfer fertig
    while(ws2812_dma_status!=0);
    // DMA starten
    p_WS2812_DMA_Start();
  #endif
}


//--------------------------------------------------------------
// interne Funktion
// loescht alle WS2812-Ketten
//--------------------------------------------------------------
void p_WS2812_clearAll(void)
{
  //-------------------------
  // alle LEDs ausschalten
  //-------------------------
  UB_WS2812_All_RGB(0xFF, WS2812_RGB_COL_OFF);
  UB_WS2812_RefreshAll();
}


//--------------------------------------------------------------
// interne Funktion
// Buffer loeschen
//--------------------------------------------------------------
void p_WS2812_clearBuff(void)
{
  uint32_t n;

  // init vom Timer Array
  for(n=0;n<WS2812_TIMER_BUF_LEN;n++) {
    WS2812_TIMER_BUF[n]=0; // 0 => kein Signal
  }
}


//--------------------------------------------------------------
// interne Funktion
// errechnet aus den RGB-Farbwerten der aktiven LEDs
// die notwendigen PWM-Werte fuer die Datenleitung
// mode : 0=normal, 1=test der laenge
//--------------------------------------------------------------
void p_WS2812_calcTimerBuf(uint8_t mode)
{
  uint32_t n;
  uint32_t pos;
  WS2812_RGB_t led;

  if(ws2812_aktiv_channel==0) return;

  pos=0;
  // timingzeiten fuer alle LEDs setzen
  for(n=0;n<ws2812_maxanz;n++) {
    led=ws2812_ptr[n];

    // Col:Green , Bit:7..0
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.green&0x80)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.green&0x40)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.green&0x20)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.green&0x10)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.green&0x08)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.green&0x04)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.green&0x02)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.green&0x01)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;

    // Col:Red , Bit:7..0
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.red&0x80)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.red&0x40)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.red&0x20)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.red&0x10)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.red&0x08)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.red&0x04)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.red&0x02)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.red&0x01)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;

    // Col:Blue , Bit:7..0
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.blue&0x80)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.blue&0x40)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.blue&0x20)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.blue&0x10)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.blue&0x08)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.blue&0x04)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.blue&0x02)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
    WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
    if((led.blue&0x01)!=0) WS2812_TIMER_BUF[pos]=WS2812_HI_TIME;
    pos++;
  } 

  if(mode==0) {
    // nach den Farbinformationen eine Pausenzeit anhaengen (2*30ms)
    for(n=0;n<WS2812_PAUSE_ANZ*WS2812_BIT_PER_LED;n++) {
      WS2812_TIMER_BUF[pos]=0;  // 0 => fuer Pausenzeit
      pos++;
    }
  }
  else {
    // zum Test der Laenge noch mehr Clocks anhaengen
    for(n=0;n<WS2812_PAUSE_ANZ*WS2812_BIT_PER_LED;n++) {
      WS2812_TIMER_BUF[pos]=WS2812_LO_TIME;
      pos++;
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// DMA und Timer starten
// (gestoppt wird per Transfer-Complete-Interrupt)
//--------------------------------------------------------------
void p_WS2812_DMA_Start(void)
{
  if(ws2812_aktiv_channel==0) return;

  // status auf "busy" setzen
  ws2812_dma_status=1;
  // init vom DMA
  p_WS2812_InitDMA();
  if(ws2812_aktiv_channel==1) {
    // enable vom Transfer-Complete Interrupt
    DMA_ITConfig(WS2812_DMA_CH1_STREAM, DMA_IT_TC, ENABLE);
    // DMA enable
    DMA_Cmd(WS2812_DMA_CH1_STREAM, ENABLE);
  }
  else if(ws2812_aktiv_channel==2) {
    // enable vom Transfer-Complete Interrupt
    DMA_ITConfig(WS2812_DMA_CH2_STREAM, DMA_IT_TC, ENABLE);
    // DMA enable
    DMA_Cmd(WS2812_DMA_CH2_STREAM, ENABLE);
  }
  else if(ws2812_aktiv_channel==3) {
    // enable vom Transfer-Complete Interrupt
    DMA_ITConfig(WS2812_DMA_CH3_STREAM, DMA_IT_TC, ENABLE);
    // DMA enable
    DMA_Cmd(WS2812_DMA_CH3_STREAM, ENABLE);
  }
  else if(ws2812_aktiv_channel==4) {
    // enable vom Transfer-Complete Interrupt
    DMA_ITConfig(WS2812_DMA_CH4_STREAM, DMA_IT_TC, ENABLE);
    // DMA enable
    DMA_Cmd(WS2812_DMA_CH4_STREAM, ENABLE);
  }
  else if(ws2812_aktiv_channel==5) {
    // enable vom Transfer-Complete Interrupt
    DMA_ITConfig(WS2812_DMA_CH5_STREAM, DMA_IT_TC, ENABLE);
    // DMA enable
    DMA_Cmd(WS2812_DMA_CH5_STREAM, ENABLE);
  }
  else if(ws2812_aktiv_channel==6) {
    // enable vom Transfer-Complete Interrupt
    DMA_ITConfig(WS2812_DMA_CH6_STREAM, DMA_IT_TC, ENABLE);
    // DMA enable
    DMA_Cmd(WS2812_DMA_CH6_STREAM, ENABLE);
  }
  else if(ws2812_aktiv_channel==7) {
    // enable vom Transfer-Complete Interrupt
    DMA_ITConfig(WS2812_DMA_CH7_STREAM, DMA_IT_TC, ENABLE);
    // DMA enable
    DMA_Cmd(WS2812_DMA_CH7_STREAM, ENABLE);
  }
  else if(ws2812_aktiv_channel==8) {
    // enable vom Transfer-Complete Interrupt
    DMA_ITConfig(WS2812_DMA_CH8_STREAM, DMA_IT_TC, ENABLE);
    // DMA enable
    DMA_Cmd(WS2812_DMA_CH8_STREAM, ENABLE);
  }
  // Timer enable
  if((ws2812_aktiv_channel>=1) && (ws2812_aktiv_channel<=4)) {
    TIM_Cmd(WS2812_TIM, ENABLE);
  }
  if((ws2812_aktiv_channel>=5) && (ws2812_aktiv_channel<=8)) {
    TIM_Cmd(WS2812_TIMB, ENABLE);
  }
}


//--------------------------------------------------------------
// interne Funktion
// init aller GPIO Pins
//--------------------------------------------------------------
void p_WS2812_InitIO(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  #if WS2812_CH1_ENABLE>0
    // Clock Enable
    RCC_AHB1PeriphClockCmd(WS2812_CH1_CLOCK, ENABLE);

    // Config des Pins als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = WS2812_CH1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(WS2812_CH1_PORT, &GPIO_InitStructure);

    // Lo-Pegel ausgeben
    WS2812_CH1_PORT->BSRRH = WS2812_CH1_PIN;

    // Alternative-Funktion mit dem IO-Pin verbinden
    GPIO_PinAFConfig(WS2812_CH1_PORT, WS2812_CH1_SOURCE, WS2812_TIM_AF);
  #endif

  #if WS2812_CH2_ENABLE>0
    // Clock Enable
    RCC_AHB1PeriphClockCmd(WS2812_CH2_CLOCK, ENABLE);

    // Config des Pins als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = WS2812_CH2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(WS2812_CH2_PORT, &GPIO_InitStructure);

    // Lo-Pegel ausgeben
    WS2812_CH2_PORT->BSRRH = WS2812_CH2_PIN;

    // Alternative-Funktion mit dem IO-Pin verbinden
    GPIO_PinAFConfig(WS2812_CH2_PORT, WS2812_CH2_SOURCE, WS2812_TIM_AF);
  #endif

  #if WS2812_CH3_ENABLE>0
    // Clock Enable
    RCC_AHB1PeriphClockCmd(WS2812_CH3_CLOCK, ENABLE);

    // Config des Pins als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = WS2812_CH3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(WS2812_CH3_PORT, &GPIO_InitStructure);

    // Lo-Pegel ausgeben
    WS2812_CH3_PORT->BSRRH = WS2812_CH3_PIN;

    // Alternative-Funktion mit dem IO-Pin verbinden
    GPIO_PinAFConfig(WS2812_CH3_PORT, WS2812_CH3_SOURCE, WS2812_TIM_AF);
  #endif

  #if WS2812_CH4_ENABLE>0
    // Clock Enable
    RCC_AHB1PeriphClockCmd(WS2812_CH4_CLOCK, ENABLE);

    // Config des Pins als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = WS2812_CH4_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(WS2812_CH4_PORT, &GPIO_InitStructure);

    // Lo-Pegel ausgeben
    WS2812_CH4_PORT->BSRRH = WS2812_CH4_PIN;

    // Alternative-Funktion mit dem IO-Pin verbinden
    GPIO_PinAFConfig(WS2812_CH4_PORT, WS2812_CH4_SOURCE, WS2812_TIM_AF);
  #endif

  #if WS2812_CH5_ENABLE>0
    // Clock Enable
    RCC_AHB1PeriphClockCmd(WS2812_CH5_CLOCK, ENABLE);

    // Config des Pins als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = WS2812_CH5_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(WS2812_CH5_PORT, &GPIO_InitStructure);

    // Lo-Pegel ausgeben
    WS2812_CH5_PORT->BSRRH = WS2812_CH5_PIN;

    // Alternative-Funktion mit dem IO-Pin verbinden
    GPIO_PinAFConfig(WS2812_CH5_PORT, WS2812_CH5_SOURCE, WS2812_TIMB_AF);
  #endif

  #if WS2812_CH6_ENABLE>0
    // Clock Enable
    RCC_AHB1PeriphClockCmd(WS2812_CH6_CLOCK, ENABLE);

    // Config des Pins als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = WS2812_CH6_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(WS2812_CH6_PORT, &GPIO_InitStructure);

    // Lo-Pegel ausgeben
    WS2812_CH6_PORT->BSRRH = WS2812_CH6_PIN;

    // Alternative-Funktion mit dem IO-Pin verbinden
    GPIO_PinAFConfig(WS2812_CH6_PORT, WS2812_CH6_SOURCE, WS2812_TIMB_AF);
  #endif

  #if WS2812_CH7_ENABLE>0
    // Clock Enable
    RCC_AHB1PeriphClockCmd(WS2812_CH7_CLOCK, ENABLE);

    // Config des Pins als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = WS2812_CH7_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(WS2812_CH7_PORT, &GPIO_InitStructure);

    // Lo-Pegel ausgeben
    WS2812_CH7_PORT->BSRRH = WS2812_CH7_PIN;

    // Alternative-Funktion mit dem IO-Pin verbinden
    GPIO_PinAFConfig(WS2812_CH7_PORT, WS2812_CH7_SOURCE, WS2812_TIMB_AF);
  #endif

  #if WS2812_CH8_ENABLE>0
    // Clock Enable
    RCC_AHB1PeriphClockCmd(WS2812_CH8_CLOCK, ENABLE);

    // Config des Pins als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = WS2812_CH8_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(WS2812_CH8_PORT, &GPIO_InitStructure);

    // Lo-Pegel ausgeben
    WS2812_CH8_PORT->BSRRH = WS2812_CH8_PIN;

    // Alternative-Funktion mit dem IO-Pin verbinden
    GPIO_PinAFConfig(WS2812_CH8_PORT, WS2812_CH8_SOURCE, WS2812_TIMB_AF);
  #endif

  //-------------------------------------
  // EXT-INT Pin
  //-------------------------------------
  RCC_AHB1PeriphClockCmd(WS2812_EXTI_CLOCK, ENABLE);

  // Config als Digital-Eingang
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin = WS2812_EXTI_PIN;
  GPIO_Init(WS2812_EXTI_PORT, &GPIO_InitStructure);

  // Clock enable (SYSCONFIG)
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  // EXT_INT0 mit Pin verbinden
  SYSCFG_EXTILineConfig(WS2812_EXTI_SOURCE, EXTI_PinSource0);
}


//--------------------------------------------------------------
// interne Funktion
// init vom Timer
//--------------------------------------------------------------
void p_WS2812_InitTIM(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  // Clock enable (TIM)
  RCC_APB1PeriphClockCmd(WS2812_TIM_CLOCK, ENABLE);

  // Clock enable (TIMB)
  RCC_APB2PeriphClockCmd(WS2812_TIMB_CLOCK, ENABLE);

  // Clock Enable (DMA)
  RCC_AHB1PeriphClockCmd(WS2812_DMA_CLOCK, ENABLE);

  // Clock Enable (DMAB)
  RCC_AHB1PeriphClockCmd(WS2812_DMAB_CLOCK, ENABLE);

  // Timer init (CH1 bis CH4)
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

  #if WS2812_CH1_ENABLE>0
    TIM_OC1Init(WS2812_TIM, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(WS2812_TIM, TIM_OCPreload_Enable);
  #endif
  #if WS2812_CH2_ENABLE>0
    TIM_OC2Init(WS2812_TIM, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(WS2812_TIM, TIM_OCPreload_Enable);
  #endif
  #if WS2812_CH3_ENABLE>0
    TIM_OC3Init(WS2812_TIM, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(WS2812_TIM, TIM_OCPreload_Enable);
  #endif
  #if WS2812_CH4_ENABLE>0
    TIM_OC4Init(WS2812_TIM, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(WS2812_TIM, TIM_OCPreload_Enable);
  #endif

  // Timer enable
  TIM_ARRPreloadConfig(WS2812_TIM, ENABLE);


  // Timer init (CH5 bis CH8)
  TIM_TimeBaseStructure.TIM_Period = WS2812_TIMB_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = WS2812_TIMB_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(WS2812_TIMB, &TIM_TimeBaseStructure);

  // deinit
  TIM_OCStructInit(&TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  #if WS2812_CH5_ENABLE>0
    TIM_OC1Init(WS2812_TIMB, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(WS2812_TIMB, TIM_OCPreload_Enable);
  #endif
  #if WS2812_CH6_ENABLE>0
    TIM_OC2Init(WS2812_TIMB, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(WS2812_TIMB, TIM_OCPreload_Enable);
  #endif
  #if WS2812_CH7_ENABLE>0
    TIM_OC3Init(WS2812_TIMB, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(WS2812_TIMB, TIM_OCPreload_Enable);
  #endif
  #if WS2812_CH8_ENABLE>0
    TIM_OC4Init(WS2812_TIMB, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(WS2812_TIMB, TIM_OCPreload_Enable);
  #endif

  // Timer enable
  TIM_ARRPreloadConfig(WS2812_TIMB, ENABLE);
  // Timer1 oder Timer8
  TIM_CtrlPWMOutputs(WS2812_TIMB, ENABLE);
}


//--------------------------------------------------------------
// interne Funktion
// init vom DMA
//--------------------------------------------------------------
void p_WS2812_InitDMA(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  if(ws2812_aktiv_channel==0) return;

  // DMA init
  if(ws2812_aktiv_channel==1) {
    DMA_Cmd(WS2812_DMA_CH1_STREAM, DISABLE);
    DMA_DeInit(WS2812_DMA_CH1_STREAM);
    DMA_InitStructure.DMA_Channel = WS2812_DMA_CH1_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIM_CCR_REG1;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (WS2812_CHAIN_LEN.ch1+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED;
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
    DMA_Init(WS2812_DMA_CH1_STREAM, &DMA_InitStructure);
  }
  else if(ws2812_aktiv_channel==2) {
    DMA_Cmd(WS2812_DMA_CH2_STREAM, DISABLE);
    DMA_DeInit(WS2812_DMA_CH2_STREAM);
    DMA_InitStructure.DMA_Channel = WS2812_DMA_CH2_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIM_CCR_REG2;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (WS2812_CHAIN_LEN.ch2+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED;
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
    DMA_Init(WS2812_DMA_CH2_STREAM, &DMA_InitStructure);
  }
  else if(ws2812_aktiv_channel==3) {
    DMA_Cmd(WS2812_DMA_CH3_STREAM, DISABLE);
    DMA_DeInit(WS2812_DMA_CH3_STREAM);
    DMA_InitStructure.DMA_Channel = WS2812_DMA_CH3_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIM_CCR_REG3;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (WS2812_CHAIN_LEN.ch3+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED;
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
    DMA_Init(WS2812_DMA_CH3_STREAM, &DMA_InitStructure);
  }
  else if(ws2812_aktiv_channel==4) {
    DMA_Cmd(WS2812_DMA_CH4_STREAM, DISABLE);
    DMA_DeInit(WS2812_DMA_CH4_STREAM);
    DMA_InitStructure.DMA_Channel = WS2812_DMA_CH4_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIM_CCR_REG4;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (WS2812_CHAIN_LEN.ch4+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED;
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
    DMA_Init(WS2812_DMA_CH4_STREAM, &DMA_InitStructure);
  }
  if(ws2812_aktiv_channel==5) {
    DMA_Cmd(WS2812_DMA_CH5_STREAM, DISABLE);
    DMA_DeInit(WS2812_DMA_CH5_STREAM);
    DMA_InitStructure.DMA_Channel = WS2812_DMA_CH5_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIMB_CCR_REG1;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (WS2812_CHAIN_LEN.ch5+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED;
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
    DMA_Init(WS2812_DMA_CH5_STREAM, &DMA_InitStructure);
  }
  else if(ws2812_aktiv_channel==6) {
    DMA_Cmd(WS2812_DMA_CH6_STREAM, DISABLE);
    DMA_DeInit(WS2812_DMA_CH6_STREAM);
    DMA_InitStructure.DMA_Channel = WS2812_DMA_CH6_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIMB_CCR_REG2;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (WS2812_CHAIN_LEN.ch6+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED;
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
    DMA_Init(WS2812_DMA_CH6_STREAM, &DMA_InitStructure);
  }
  else if(ws2812_aktiv_channel==7) {
    DMA_Cmd(WS2812_DMA_CH7_STREAM, DISABLE);
    DMA_DeInit(WS2812_DMA_CH7_STREAM);
    DMA_InitStructure.DMA_Channel = WS2812_DMA_CH7_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIMB_CCR_REG3;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (WS2812_CHAIN_LEN.ch7+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED;
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
    DMA_Init(WS2812_DMA_CH7_STREAM, &DMA_InitStructure);
  }
  else if(ws2812_aktiv_channel==8) {
    DMA_Cmd(WS2812_DMA_CH8_STREAM, DISABLE);
    DMA_DeInit(WS2812_DMA_CH8_STREAM);
    DMA_InitStructure.DMA_Channel = WS2812_DMA_CH8_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &WS2812_TIMB_CCR_REG4;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)WS2812_TIMER_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (WS2812_CHAIN_LEN.ch8+WS2812_PAUSE_ANZ)*WS2812_BIT_PER_LED;
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
    DMA_Init(WS2812_DMA_CH8_STREAM, &DMA_InitStructure);
  }
}


//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void p_WS2812_InitNVIC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  //-----------------
  // CH1 bis CH4
  //-----------------

  #if WS2812_CH1_ENABLE>0
    TIM_DMACmd(WS2812_TIM, WS2812_TIM_DMA_TRG1, ENABLE);
  #endif
  #if WS2812_CH2_ENABLE>0
    TIM_DMACmd(WS2812_TIM, WS2812_TIM_DMA_TRG2, ENABLE);
  #endif
  #if WS2812_CH3_ENABLE>0
    TIM_DMACmd(WS2812_TIM, WS2812_TIM_DMA_TRG3, ENABLE);
  #endif
  #if WS2812_CH4_ENABLE>0
    TIM_DMACmd(WS2812_TIM, WS2812_TIM_DMA_TRG4, ENABLE);
  #endif

  #if WS2812_CH1_ENABLE>0
    NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_CH1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  #endif
  #if WS2812_CH2_ENABLE>0
    NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_CH2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  #endif
  #if WS2812_CH3_ENABLE>0
    NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_CH3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  #endif
  #if WS2812_CH4_ENABLE>0
    NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_CH4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  #endif

  //-----------------
  // CH5 bis CH8
  //-----------------

  #if WS2812_CH5_ENABLE>0
    TIM_DMACmd(WS2812_TIMB, WS2812_TIMB_DMA_TRG1, ENABLE);
  #endif
  #if WS2812_CH6_ENABLE>0
    TIM_DMACmd(WS2812_TIMB, WS2812_TIMB_DMA_TRG2, ENABLE);
  #endif
  #if WS2812_CH7_ENABLE>0
    TIM_DMACmd(WS2812_TIMB, WS2812_TIMB_DMA_TRG3, ENABLE);
  #endif
  #if WS2812_CH8_ENABLE>0
    TIM_DMACmd(WS2812_TIMB, WS2812_TIMB_DMA_TRG4, ENABLE);
  #endif

  #if WS2812_CH5_ENABLE>0
    NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_CH5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  #endif
  #if WS2812_CH6_ENABLE>0
    NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_CH6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  #endif
  #if WS2812_CH7_ENABLE>0
    NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_CH7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  #endif
  #if WS2812_CH8_ENABLE>0
    NVIC_InitStructure.NVIC_IRQChannel = WS2812_DMA_CH8_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  #endif


  // NVIC fuer EXTI
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// init vom EXTI
//--------------------------------------------------------------
void p_WS2812_InitEXTI(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;

  // EXT_INT0 config
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// ISR vom DMA (CH1)
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
//--------------------------------------------------------------
void WS2812_DMA_CH1_ISR(void)
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_CH1_STREAM, WS2812_DMA_CH1_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_CH1_STREAM, WS2812_DMA_CH1_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIM, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_CH1_STREAM, DISABLE);
    
    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}


//--------------------------------------------------------------
// interne Funktion
// ISR vom DMA (CH2)
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
//--------------------------------------------------------------
void WS2812_DMA_CH2_ISR(void)
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_CH2_STREAM, WS2812_DMA_CH2_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_CH2_STREAM, WS2812_DMA_CH2_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIM, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_CH2_STREAM, DISABLE);

    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}


//--------------------------------------------------------------
// interne Funktion
// ISR vom DMA (CH3)
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
//--------------------------------------------------------------
void WS2812_DMA_CH3_ISR(void)
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_CH3_STREAM, WS2812_DMA_CH3_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_CH3_STREAM, WS2812_DMA_CH3_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIM, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_CH3_STREAM, DISABLE);

    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}


//--------------------------------------------------------------
// interne Funktion
// ISR vom DMA (CH4)
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
//--------------------------------------------------------------
void WS2812_DMA_CH4_ISR(void)
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_CH4_STREAM, WS2812_DMA_CH4_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_CH4_STREAM, WS2812_DMA_CH4_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIM, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_CH4_STREAM, DISABLE);

    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}


//--------------------------------------------------------------
// interne Funktion
// ISR vom DMA (CH5)
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
//--------------------------------------------------------------
void WS2812_DMA_CH5_ISR(void)
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_CH5_STREAM, WS2812_DMA_CH5_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_CH5_STREAM, WS2812_DMA_CH5_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIMB, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_CH5_STREAM, DISABLE);

    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}


//--------------------------------------------------------------
// interne Funktion
// ISR vom DMA (CH6)
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
//--------------------------------------------------------------
void WS2812_DMA_CH6_ISR(void)
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_CH6_STREAM, WS2812_DMA_CH6_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_CH6_STREAM, WS2812_DMA_CH6_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIMB, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_CH6_STREAM, DISABLE);

    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}


//--------------------------------------------------------------
// interne Funktion
// ISR vom DMA (CH7)
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
//--------------------------------------------------------------
void WS2812_DMA_CH7_ISR(void)
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_CH7_STREAM, WS2812_DMA_CH7_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_CH7_STREAM, WS2812_DMA_CH7_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIMB, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_CH7_STREAM, DISABLE);

    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}


//--------------------------------------------------------------
// interne Funktion
// ISR vom DMA (CH8)
// (wird aufgerufen, wenn alle Daten uebertragen wurden)
//--------------------------------------------------------------
void WS2812_DMA_CH8_ISR(void)
{
  // Test auf Transfer-Complete Interrupt Flag
  if (DMA_GetITStatus(WS2812_DMA_CH8_STREAM, WS2812_DMA_CH8_IRQ_FLAG))
  {
    // Flag zuruecksetzen
    DMA_ClearITPendingBit(WS2812_DMA_CH8_STREAM, WS2812_DMA_CH8_IRQ_FLAG);

    // Timer disable
    TIM_Cmd(WS2812_TIMB, DISABLE);
    // DMA disable
    DMA_Cmd(WS2812_DMA_CH8_STREAM, DISABLE);

    // status auf "ready" setzen
    ws2812_dma_status=0;
  }
}


//--------------------------------------------------------------
// external Interrupt-0
// wird aufgerufen, wenn das erste Clock-Signal
// durch alle WS2812-LEDs geschoben wurde
//--------------------------------------------------------------
void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
    // wenn Interrupt aufgetreten
    EXTI_ClearITPendingBit(EXTI_Line0);

    if(ws2812_len==0) {
      if(ws2812_aktiv_channel==1) ws2812_len=WS2812_TIMER_BUF_LEN-WS2812_DMA_CH1_STREAM->NDTR;
      if(ws2812_aktiv_channel==2) ws2812_len=WS2812_TIMER_BUF_LEN-WS2812_DMA_CH2_STREAM->NDTR;
      if(ws2812_aktiv_channel==3) ws2812_len=WS2812_TIMER_BUF_LEN-WS2812_DMA_CH3_STREAM->NDTR;
      if(ws2812_aktiv_channel==4) ws2812_len=WS2812_TIMER_BUF_LEN-WS2812_DMA_CH4_STREAM->NDTR;
      if(ws2812_aktiv_channel==5) ws2812_len=WS2812_TIMER_BUF_LEN-WS2812_DMA_CH5_STREAM->NDTR;
      if(ws2812_aktiv_channel==6) ws2812_len=WS2812_TIMER_BUF_LEN-WS2812_DMA_CH6_STREAM->NDTR;
      if(ws2812_aktiv_channel==7) ws2812_len=WS2812_TIMER_BUF_LEN-WS2812_DMA_CH7_STREAM->NDTR;
      if(ws2812_aktiv_channel==8) ws2812_len=WS2812_TIMER_BUF_LEN-WS2812_DMA_CH8_STREAM->NDTR;
    }
  }
}
