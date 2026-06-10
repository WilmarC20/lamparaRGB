#ifndef UI_CONTROL_SCREEN_H
#define UI_CONTROL_SCREEN_H

#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_Control_screen_init(void);
void ui_set_active_tab(uint8_t tab);
uint8_t ui_get_active_tab(void);
bool ui_is_settings_tab(void);
void ui_update_brightness_label(uint8_t brightness);
void ui_update_speed_label(uint8_t pct);
void ui_update_wifi_icon(bool online);
void ui_highlight_effect(uint16_t idx);
void ui_update_direction_buttons(bool reverse);
lv_obj_t *ui_get_effect_list(void);
lv_obj_t *ui_get_color_tab(void);
lv_obj_t *ui_get_dir_btn_left(void);
lv_obj_t *ui_get_dir_btn_right(void);
void ui_set_settings_brightness_label(lv_obj_t *lbl);
void ui_update_color_preview(uint16_t hue, uint8_t sat);
void ui_bind_nav_callbacks(lv_event_cb_t cb);
void ui_bind_preset_callbacks(lv_event_cb_t cb);
void ui_bind_effect_callbacks(lv_event_cb_t cb);

void ui_control_register_callbacks(lv_event_cb_t effect_cb,
                                   lv_event_cb_t preset_cb,
                                   lv_event_cb_t nav_cb,
                                   lv_event_cb_t speed_cb,
                                   lv_event_cb_t dir_cb,
                                   void (*settings_bind_fn)(void));

#ifdef __cplusplus
}
#endif

#endif
