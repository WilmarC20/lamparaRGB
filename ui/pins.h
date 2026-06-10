#ifndef LAMPARA_PINS_H
#define LAMPARA_PINS_H

/*
 * ESP32-2432S024C (2.4" capacitive, CST820)
 */

/* Display TFT_eSPI — ILI9341 (HSPI) */
#define PIN_TFT_MISO 12
#define PIN_TFT_MOSI 13
#define PIN_TFT_SCLK 14
#define PIN_TFT_CS   15
#define PIN_TFT_DC   2
#define PIN_TFT_RST  -1
#define PIN_TFT_BL   27

/* Touch CST820 — I2C */
#define PIN_TP_SDA 33
#define PIN_TP_SCL 32
#define PIN_TP_INT 21
#define PIN_TP_RST 25

/*
 * Conectores JST:
 *   CN1: GND | IO27 | IO22 | 3V3
 *   P3:  GND | IO35 | IO22 | IO21
 *   P1:  +5V | TXD(1) | RXD(3) | GND
 *
 * Cableado que FUNCIONO (IR en CN1):
 *   WS2812 DIN  -> P1  TXD  (GPIO 1)   + 5V/GND P1
 *   TSOP4838 OUT -> CN1 IO22 (GPIO 22)
 *   MAX9814 OUT  -> P3  IO35 (GPIO 35)
 *
 * GPIO 3 (P1 RXD) no sirve bien para IR (UART USB).
 * GPIO 1 = WS2812 + Serial TX (consola legible solo si LEDs pausados / plotter mic).
 */

#define PIN_LED_DATA 1
#define LED_COUNT    90   /* 3 niveles x 30, espiral en carretes PLA 1 kg */

#include "led_layout.h"

#define PIN_MIC_ADC  35

#define PIN_IR       22

/* Altavoz onboard (JST P4) -> FM8002A, DAC2 */
#define PIN_SPEAKER  26

#endif
