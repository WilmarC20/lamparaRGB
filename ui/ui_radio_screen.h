#ifndef UI_RADIO_SCREEN_H
#define UI_RADIO_SCREEN_H

#include "lvgl.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t *ui_radio_tab_build(lv_obj_t *parent);
void ui_radio_bind_station_cb(lv_event_cb_t cb);
void ui_radio_bind_stop_cb(lv_event_cb_t cb);
void ui_radio_bind_play_cb(lv_event_cb_t cb);
void ui_radio_bind_volume_cb(lv_event_cb_t cb);
void ui_radio_highlight_station(int idx);
void ui_radio_update_now_playing(const char *station, const char *status);
void ui_radio_set_volume_pct(uint8_t pct);
void ui_radio_update_play_btn(bool playing);

#ifdef __cplusplus
}
#endif

#endif
