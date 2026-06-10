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
typedef enum {
    UI_WIFI_DISCONNECTED = 0,
    UI_WIFI_CONNECTING,
    UI_WIFI_CONNECTED,
} ui_wifi_state_t;

void ui_update_wifi_status(ui_wifi_state_t state, const char *ssid);
void ui_update_header_clock(const char *time_text);
void ui_highlight_effect(uint16_t idx);
void ui_update_direction_buttons(bool reverse);
lv_obj_t *ui_get_effect_list(void);
lv_obj_t *ui_get_effect_color_bar(void);
lv_obj_t *ui_get_color_tab(void);
lv_obj_t *ui_get_dir_btn_left(void);
lv_obj_t *ui_get_dir_btn_right(void);
void ui_set_settings_brightness_label(lv_obj_t *lbl);
void ui_update_color_preview(uint16_t hue, uint8_t sat);
void ui_update_effect_color_bar(uint16_t effect_idx, uint16_t hue, uint8_t sat);
void ui_bind_nav_callbacks(lv_event_cb_t cb);
void ui_bind_preset_callbacks(lv_event_cb_t cb);
void ui_bind_effect_callbacks(lv_event_cb_t cb);

void ui_control_register_effect_color_cb(lv_event_cb_t cb);
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
