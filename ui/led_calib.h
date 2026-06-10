#ifndef LED_CALIB_H
#define LED_CALIB_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool led_calib_is_active(void);
void led_calib_set_active(bool active);
uint16_t led_calib_get_index(void);
uint16_t led_calib_get_count(void);
bool led_calib_step(int delta);
void led_calib_set_index(uint16_t idx);

#ifdef __cplusplus
}
#endif

#endif
