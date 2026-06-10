#ifndef UI_CONFIG_SCREEN_H
#define UI_CONFIG_SCREEN_H

#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_Config_screen_init(void);
lv_obj_t *ui_settings_tab_build(lv_obj_t *parent);
void ui_config_set_wifi_status(bool online);
lv_obj_t *ui_config_get_brightness_slider(void);
void ui_config_set_status(const char *text, uint32_t color_hex);
void ui_config_set_mic_test_active(bool active);
void ui_config_update_mic_readings(int raw, int level, int span);
void ui_config_set_led_calib_active(bool active);
void ui_config_update_led_calib(uint16_t index, uint16_t total);

#ifdef __cplusplus
}
#endif

#endif
