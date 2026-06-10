#ifndef LED_LAYOUT_H
#define LED_LAYOUT_H

#include <stdint.h>

/*
 * Tira WS2812 en espiral dentro de 3 cilindros (carretes PLA 1 kg).
 * Indice 0 = inicio de cinta (data in); sube en espiral por los 3 niveles.
 *
 *   Nivel 0 (bajo):  LED  0 .. 29
 *   Nivel 1 (medio): LED 30 .. 59
 *   Nivel 2 (alto):  LED 60 .. 89
 */

#define LED_LEVEL_COUNT  3
#define LEDS_PER_LEVEL   30

static inline uint8_t led_level_of(uint16_t index)
{
    return (uint8_t)(index / LEDS_PER_LEVEL);
}

static inline uint16_t led_index_in_level(uint16_t index)
{
    return (uint16_t)(index % LEDS_PER_LEVEL);
}

static inline uint16_t led_level_start(uint8_t level)
{
    if (level >= LED_LEVEL_COUNT) {
        level = (uint8_t)(LED_LEVEL_COUNT - 1);
    }
    return (uint16_t)(level * LEDS_PER_LEVEL);
}

#endif
