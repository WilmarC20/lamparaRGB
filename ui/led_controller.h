#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "lamp_state.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void led_controller_init(void);
void led_controller_apply(lamp_state_t *state);
void led_controller_service(void);
void led_controller_set_party_pixels(const uint8_t *rgb, uint16_t count);
/** Pausa WS2812 (GPIO 1) para Serial legible mientras plotter activo */
void led_controller_set_output_paused(bool paused);
bool led_controller_output_paused(void);
const char *led_effect_name(lamp_effect_t effect);
void led_controller_set_speed(uint16_t speed);
void led_controller_set_reverse(bool reverse);
uint16_t led_controller_get_speed(void);
bool led_controller_get_reverse(void);

#ifdef __cplusplus
}
#endif

#endif
