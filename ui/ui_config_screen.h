#ifndef UI_CONFIG_SCREEN_H
#define UI_CONFIG_SCREEN_H

#include "lvgl.h"
#include "ui_control_screen.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_Config_screen_init(void);
lv_obj_t *ui_settings_tab_build(lv_obj_t *parent);
void ui_config_set_wifi_status(bool online);
void ui_config_set_wifi_text(const char *text, bool online);
void ui_config_set_wifi_state(ui_wifi_state_t state, const char *text);
lv_obj_t *ui_config_get_brightness_slider(void);
lv_obj_t *ui_config_get_mic_sensitivity_slider(void);
void ui_config_set_status(const char *text, uint32_t color_hex);
void ui_config_set_mic_sensitivity_pct(uint8_t pct);
void ui_config_update_mic_sensitivity_meter(int sound_level, int music_level);
lv_obj_t *ui_config_get_timer_row(void);
lv_obj_t *ui_config_get_night_switch(void);
void ui_config_refresh_timer_label(void);
void ui_config_sync_night_switch(bool enabled);

#ifdef __cplusplus
}
#endif

#endif
