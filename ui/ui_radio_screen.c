#include "ui_radio_screen.h"
#include "ui_theme.h"
#include "config.h"
#include "radio_player.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdint.h>

#if ENABLE_RADIO

static lv_obj_t *s_nowPlayingLbl = NULL;
static lv_obj_t *s_statusLbl = NULL;
static lv_obj_t *s_stationList = NULL;
static lv_obj_t *s_stopBtn = NULL;
static lv_obj_t *s_playBtn = NULL;
static lv_obj_t *s_volSlider = NULL;
static lv_obj_t *s_volPctLbl = NULL;
static lv_event_cb_t s_cbStation = NULL;
static lv_event_cb_t s_cbStop = NULL;
static lv_event_cb_t s_cbPlay = NULL;
static lv_event_cb_t s_cbVolume = NULL;

static lv_obj_t *make_station_btn(lv_obj_t *parent, uint8_t idx)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 104, 26);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(UI_COLOR_ACCENT), LV_STATE_CHECKED);
    lv_obj_set_style_border_color(btn, lv_color_hex(UI_COLOR_CARD_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(btn, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn, 4, LV_PART_MAIN);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_t *ico = lv_label_create(btn);
    lv_label_set_text(ico, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_color(ico, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_color(ico, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, radio_player_station_name(idx));
    lv_obj_set_style_text_color(lbl, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_flex_grow(lbl, 1);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);

    if (s_cbStation) {
        lv_obj_add_event_cb(btn, s_cbStation, LV_EVENT_CLICKED, (void *)(intptr_t)(idx + 1U));
    }

    return btn;
}

static lv_obj_t *make_transport_btn(lv_obj_t *parent, const char *sym, lv_coord_t w)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, 32);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(UI_COLOR_ACCENT), LV_STATE_CHECKED);
    lv_obj_set_style_border_color(btn, lv_color_hex(UI_COLOR_CARD_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, sym);
    lv_obj_center(lbl);
    return btn;
}

lv_obj_t *ui_radio_tab_build(lv_obj_t *parent)
{
    lv_obj_t *tab = lv_obj_create(parent);
    lv_obj_set_size(tab, kScreenWidth, UI_CONTENT_H);
    lv_obj_align(tab, LV_ALIGN_TOP_MID, 0, UI_HDR_H);
    ui_style_screen(tab);
    lv_obj_add_flag(tab, LV_OBJ_FLAG_HIDDEN);

    /* Izquierda: reproductor + volumen (188 px, como panel derecho de EFX) */
    lv_obj_t *leftCol = lv_obj_create(tab);
    lv_obj_set_size(leftCol, 188, UI_CONTENT_H - 8);
    lv_obj_set_pos(leftCol, 4, 4);
    lv_obj_set_style_bg_opa(leftCol, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(leftCol, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(leftCol, 0, LV_PART_MAIN);
    lv_obj_clear_flag(leftCol, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *playerPanel = lv_obj_create(leftCol);
    lv_obj_set_size(playerPanel, 184, 110);
    lv_obj_set_pos(playerPanel, 0, 0);
    ui_style_card(playerPanel);
    lv_obj_clear_flag(playerPanel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *hdrIco = lv_label_create(playerPanel);
    lv_label_set_text(hdrIco, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_color(hdrIco, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align(hdrIco, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *hdrLbl = lv_label_create(playerPanel);
    lv_label_set_text(hdrLbl, "REPRODUCTOR");
    lv_obj_set_style_text_color(hdrLbl, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align_to(hdrLbl, hdrIco, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    s_nowPlayingLbl = lv_label_create(playerPanel);
    lv_label_set_text(s_nowPlayingLbl, "Selecciona emisora");
    lv_obj_set_style_text_color(s_nowPlayingLbl, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_width(s_nowPlayingLbl, 168);
    lv_label_set_long_mode(s_nowPlayingLbl, LV_LABEL_LONG_DOT);
    lv_obj_align(s_nowPlayingLbl, LV_ALIGN_TOP_LEFT, 0, 18);

    s_statusLbl = lv_label_create(playerPanel);
    lv_label_set_text(s_statusLbl, "Detenido");
    lv_obj_set_style_text_color(s_statusLbl, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align(s_statusLbl, LV_ALIGN_TOP_LEFT, 0, 36);

    s_playBtn = make_transport_btn(playerPanel, LV_SYMBOL_PLAY " Play", 80);
    lv_obj_align(s_playBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    if (s_cbPlay) {
        lv_obj_add_event_cb(s_playBtn, s_cbPlay, LV_EVENT_CLICKED, NULL);
    }

    s_stopBtn = make_transport_btn(playerPanel, LV_SYMBOL_STOP " Stop", 80);
    lv_obj_align(s_stopBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    if (s_cbStop) {
        lv_obj_add_event_cb(s_stopBtn, s_cbStop, LV_EVENT_CLICKED, NULL);
    }

    /* 114 + 56 = 170 <= 172 (alto de la columna): sin solape con la nav bar */
    lv_obj_t *volPanel = lv_obj_create(leftCol);
    lv_obj_set_size(volPanel, 184, 56);
    lv_obj_set_pos(volPanel, 0, 114);
    ui_style_card(volPanel);
    lv_obj_clear_flag(volPanel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *volIco = lv_label_create(volPanel);
    lv_label_set_text(volIco, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_color(volIco, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align(volIco, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *volTitle = lv_label_create(volPanel);
    lv_label_set_text(volTitle, "VOLUMEN");
    lv_obj_set_style_text_color(volTitle, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align_to(volTitle, volIco, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    s_volPctLbl = lv_label_create(volPanel);
    {
        char volBuf[8];
        snprintf(volBuf, sizeof(volBuf), "%u%%", (unsigned)RADIO_DEFAULT_VOLUME_PCT);
        lv_label_set_text(s_volPctLbl, volBuf);
    }
    lv_obj_set_style_text_color(s_volPctLbl, lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_align(s_volPctLbl, LV_ALIGN_TOP_RIGHT, 0, 0);

    s_volSlider = lv_slider_create(volPanel);
    lv_slider_set_range(s_volSlider, 0, 100);
    lv_slider_set_value(s_volSlider, RADIO_DEFAULT_VOLUME_PCT, LV_ANIM_OFF);
    lv_obj_set_width(s_volSlider, 168);
    lv_obj_align(s_volSlider, LV_ALIGN_BOTTOM_MID, 0, 0);
    ui_style_slider(s_volSlider);
    if (s_cbVolume) {
        lv_obj_add_event_cb(s_volSlider, s_cbVolume, LV_EVENT_VALUE_CHANGED, NULL);
    }

    /* Derecha: lista emisoras (112 px, misma columna que lista EFX) */
    s_stationList = lv_obj_create(tab);
    lv_obj_set_size(s_stationList, 112, UI_CONTENT_H - 8);
    lv_obj_set_pos(s_stationList, 196, 4);
    lv_obj_set_style_bg_opa(s_stationList, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_stationList, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_stationList, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(s_stationList, 3, LV_PART_MAIN);
    lv_obj_set_flex_flow(s_stationList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_stationList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(s_stationList, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_stationList, LV_SCROLLBAR_MODE_AUTO);

    const uint8_t count = radio_player_station_count();
    for (uint8_t i = 0; i < count; i++) {
        make_station_btn(s_stationList, i);
    }

    return tab;
}

void ui_radio_bind_station_cb(lv_event_cb_t cb)
{
    s_cbStation = cb;
    if (!s_stationList || !cb) {
        return;
    }
    const uint32_t n = lv_obj_get_child_cnt(s_stationList);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(s_stationList, i);
        if (btn) {
            lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, (void *)(intptr_t)(i + 1U));
        }
    }
}

void ui_radio_bind_stop_cb(lv_event_cb_t cb)
{
    s_cbStop = cb;
    if (s_stopBtn && cb) {
        lv_obj_add_event_cb(s_stopBtn, cb, LV_EVENT_CLICKED, NULL);
    }
}

void ui_radio_bind_play_cb(lv_event_cb_t cb)
{
    s_cbPlay = cb;
    if (s_playBtn && cb) {
        lv_obj_add_event_cb(s_playBtn, cb, LV_EVENT_CLICKED, NULL);
    }
}

void ui_radio_bind_volume_cb(lv_event_cb_t cb)
{
    s_cbVolume = cb;
    if (s_volSlider && cb) {
        lv_obj_add_event_cb(s_volSlider, cb, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

void ui_radio_set_volume_pct(uint8_t pct)
{
    if (pct > 100) {
        pct = 100;
    }
    if (s_volSlider) {
        lv_slider_set_value(s_volSlider, pct, LV_ANIM_OFF);
    }
    if (s_volPctLbl) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%u%%", (unsigned)pct);
        lv_label_set_text(s_volPctLbl, buf);
    }
}

void ui_radio_update_play_btn(bool playing)
{
    if (!s_playBtn) {
        return;
    }
    if (playing) {
        lv_obj_add_state(s_playBtn, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(s_playBtn, LV_STATE_CHECKED);
    }
}

void ui_radio_highlight_station(int idx)
{
    if (!s_stationList) {
        return;
    }
    const uint32_t n = lv_obj_get_child_cnt(s_stationList);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(s_stationList, i);
        if (!btn) {
            continue;
        }
        lv_obj_t *ico = lv_obj_get_child(btn, 0);
        lv_obj_t *lbl = lv_obj_get_child(btn, 1);
        if ((int)i == idx) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            if (ico) {
                lv_obj_add_state(ico, LV_STATE_CHECKED);
            }
            if (lbl) {
                lv_obj_add_state(lbl, LV_STATE_CHECKED);
                lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
                lv_obj_set_style_anim_speed(lbl, 18, LV_PART_MAIN);
            }
        } else {
            lv_obj_clear_state(btn, LV_STATE_CHECKED);
            if (ico) {
                lv_obj_clear_state(ico, LV_STATE_CHECKED);
            }
            if (lbl) {
                lv_obj_clear_state(lbl, LV_STATE_CHECKED);
                lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
            }
        }
    }
}

void ui_radio_update_now_playing(const char *station, const char *status)
{
    if (s_nowPlayingLbl) {
        lv_label_set_text(s_nowPlayingLbl, (station && station[0]) ? station : "Selecciona emisora");
    }
    if (s_statusLbl && status) {
        lv_label_set_text(s_statusLbl, status);
    }
}

#else

lv_obj_t *ui_radio_tab_build(lv_obj_t *parent)
{
    (void)parent;
    return NULL;
}

void ui_radio_bind_station_cb(lv_event_cb_t cb) { (void)cb; }
void ui_radio_bind_stop_cb(lv_event_cb_t cb) { (void)cb; }
void ui_radio_bind_play_cb(lv_event_cb_t cb) { (void)cb; }
void ui_radio_bind_volume_cb(lv_event_cb_t cb) { (void)cb; }
void ui_radio_highlight_station(int idx) { (void)idx; }
void ui_radio_set_volume_pct(uint8_t pct) { (void)pct; }
void ui_radio_update_play_btn(bool playing) { (void)playing; }
void ui_radio_update_now_playing(const char *station, const char *status)
{
    (void)station;
    (void)status;
}

#endif
