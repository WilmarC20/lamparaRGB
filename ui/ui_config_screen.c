#include "ui_config_screen.h"
#include "ui_settings.h"
#include "config.h"
#include "ui_control_screen.h"
#include "ui_theme.h"
#include "lampara_ui.h"
#include "audio_input.h"
#include "music_effects.h"
#include "lvgl.h"
#include <stdio.h>

#define MIC_BAR_MAX 400
#define FW_VERSION  "Lampara V3.0.1"

static lv_obj_t *s_configStatusLbl = NULL;
static lv_obj_t *s_wifiLbl = NULL;
static lv_obj_t *s_wifiIcon = NULL;
static lv_obj_t *s_settingsBrilloSlider = NULL;
static lv_obj_t *s_settingsBrilloPct = NULL;
static lv_obj_t *s_micSensSlider = NULL;
static lv_obj_t *s_micSensPct = NULL;
static lv_obj_t *s_micSensBar = NULL;
static lv_obj_t *s_micSensLevelLbl = NULL;
static lv_obj_t *s_timerRow = NULL;
static lv_obj_t *s_timerVal = NULL;
static lv_obj_t *s_sunriseRow = NULL;
static lv_obj_t *s_sunriseVal = NULL;
static lv_obj_t *s_paletteRow = NULL;
static lv_obj_t *s_paletteVal = NULL;
static lv_obj_t *s_speakerRow = NULL;
static lv_obj_t *s_nightSw = NULL;

static lv_obj_t *make_settings_row(lv_obj_t *parent, const char *icon, uint32_t icon_color,
                                    const char *title, lv_obj_t **valueOut, int h, bool chevron,
                                    bool clickable)
{
    lv_obj_t *row = clickable ? lv_btn_create(parent) : lv_obj_create(parent);
    lv_obj_set_width(row, kScreenWidth - 16);
    lv_obj_set_height(row, h);
    ui_style_card(row);
    lv_obj_set_style_shadow_width(row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(row, 10, LV_PART_MAIN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    if (!clickable) {
        lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);
    }

    lv_obj_t *ico = lv_label_create(row);
    lv_label_set_text(ico, icon);
    lv_obj_set_style_text_color(ico, lv_color_hex(icon_color), LV_PART_MAIN);
    lv_obj_align(ico, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_color(lbl, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align_to(lbl, ico, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    lv_obj_t *val = lv_label_create(row);
    lv_label_set_text(val, "");
    lv_obj_set_style_text_color(val, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align(val, LV_ALIGN_RIGHT_MID, chevron ? -14 : -36, 0);
    if (valueOut) {
        *valueOut = val;
    }

    if (chevron) {
        lv_obj_t *chev = lv_label_create(row);
        lv_label_set_text(chev, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_color(chev, lv_color_hex(0x666677), LV_PART_MAIN);
        lv_obj_align(chev, LV_ALIGN_RIGHT_MID, 0, 0);
    }

    return row;
}

lv_obj_t *ui_settings_tab_build(lv_obj_t *parent)
{
    lv_obj_t *tab = lv_obj_create(parent);
    lv_obj_set_size(tab, kScreenWidth, UI_CONTENT_H);
    lv_obj_align(tab, LV_ALIGN_TOP_MID, 0, UI_HDR_H);
    ui_style_screen(tab);
    lv_obj_add_flag(tab, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *list = lv_obj_create(tab);
    lv_obj_set_size(list, kScreenWidth, UI_CONTENT_H);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(list, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_row(list, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);

    /* WiFi */
    lv_obj_t *wifiRow = make_settings_row(list, LV_SYMBOL_WIFI, 0x666677, "WiFi",
                                          &s_wifiLbl, 36, true, true);
    s_wifiIcon = lv_obj_get_child(wifiRow, 0);
    lv_label_set_text(s_wifiLbl, "Sin conexion");
#if ENABLE_RAINMAKER
    ui_BtnStartProv = wifiRow;
#else
    lv_obj_add_state(wifiRow, LV_STATE_DISABLED);
#endif

    /* Brillo global */
    lv_obj_t *brRow = lv_obj_create(list);
    lv_obj_set_width(brRow, kScreenWidth - 16);
    lv_obj_set_height(brRow, 54);
    ui_style_card(brRow);
    lv_obj_clear_flag(brRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(brRow, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *sunIco = lv_obj_create(brRow);
    lv_obj_set_size(sunIco, 14, 14);
    lv_obj_set_style_radius(sunIco, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sunIco, lv_color_hex(0xFFCC33), LV_PART_MAIN);
    lv_obj_set_style_border_width(sunIco, 0, LV_PART_MAIN);
    lv_obj_align(sunIco, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(sunIco, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *brTitle = lv_label_create(brRow);
    lv_label_set_text(brTitle, "Brillo global");
    lv_obj_set_style_text_color(brTitle, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align_to(brTitle, sunIco, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    s_settingsBrilloPct = lv_label_create(brRow);
    lv_label_set_text(s_settingsBrilloPct, "50%");
    lv_obj_set_style_text_color(s_settingsBrilloPct, lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_align(s_settingsBrilloPct, LV_ALIGN_TOP_RIGHT, 0, 0);
    ui_set_settings_brightness_label(s_settingsBrilloPct);

    s_settingsBrilloSlider = lv_slider_create(brRow);
    lv_slider_set_range(s_settingsBrilloSlider, 0, 255);
    lv_slider_set_value(s_settingsBrilloSlider, 128, LV_ANIM_OFF);
    lv_obj_set_width(s_settingsBrilloSlider, kScreenWidth - 40);
    lv_obj_align(s_settingsBrilloSlider, LV_ALIGN_BOTTOM_MID, 0, 0);
    ui_style_slider(s_settingsBrilloSlider);

    /* Sensibilidad microfono */
    lv_obj_t *micRow = lv_obj_create(list);
    lv_obj_set_width(micRow, kScreenWidth - 16);
    lv_obj_set_height(micRow, 68);
    ui_style_card(micRow);
    lv_obj_clear_flag(micRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(micRow, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *micIco = lv_label_create(micRow);
    lv_label_set_text(micIco, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_color(micIco, lv_color_hex(0x00CCCC), LV_PART_MAIN);
    lv_obj_align(micIco, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *micTitle = lv_label_create(micRow);
    lv_label_set_text(micTitle, "Sensibilidad mic");
    lv_obj_set_style_text_color(micTitle, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align_to(micTitle, micIco, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    s_micSensLevelLbl = lv_label_create(micRow);
    lv_label_set_text(s_micSensLevelLbl, "L 0");
    lv_obj_set_style_text_color(s_micSensLevelLbl, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align(s_micSensLevelLbl, LV_ALIGN_TOP_RIGHT, 0, 0);

    s_micSensPct = lv_label_create(micRow);
    lv_label_set_text(s_micSensPct, "50%");
    lv_obj_set_style_text_color(s_micSensPct, lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_align(s_micSensPct, LV_ALIGN_TOP_RIGHT, 0, 16);

    s_micSensSlider = lv_slider_create(micRow);
    lv_slider_set_range(s_micSensSlider, 0, 100);
    {
        const uint8_t micPct =
            audio_input_sensitivity_pct_from_silence(audio_input_get_silence_level());
        lv_slider_set_value(s_micSensSlider, micPct, LV_ANIM_OFF);
        if (s_micSensPct) {
            char pctBuf[8];
            snprintf(pctBuf, sizeof(pctBuf), "%u%%", micPct);
            lv_label_set_text(s_micSensPct, pctBuf);
        }
    }
    lv_obj_set_width(s_micSensSlider, kScreenWidth - 40);
    lv_obj_set_pos(s_micSensSlider, 0, 24);
    ui_style_slider(s_micSensSlider);
    ui_MicSensSlider = s_micSensSlider;

    s_micSensBar = lv_bar_create(micRow);
    lv_bar_set_range(s_micSensBar, 0, MIC_BAR_MAX);
    lv_bar_set_value(s_micSensBar, 0, LV_ANIM_OFF);
    lv_obj_set_size(s_micSensBar, kScreenWidth - 40, 8);
    lv_obj_align(s_micSensBar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(s_micSensBar, lv_color_hex(0x2A2A35), LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_micSensBar, lv_color_hex(0x00CCCC), LV_PART_INDICATOR);
    ui_MicSensBar = s_micSensBar;

    /* Temporizador */
    s_timerRow = make_settings_row(list, LV_SYMBOL_BELL, UI_COLOR_TEXT_DIM,
                                   "Temporizador", &s_timerVal, 36, true, true);
    ui_config_refresh_timer_label();

    /* Amanecer: rampa calida de 15 min terminando a la hora elegida */
    s_sunriseRow = make_settings_row(list, LV_SYMBOL_UP, 0xFFD500,
                                     "Amanecer", &s_sunriseVal, 36, true, true);
    ui_config_refresh_sunrise_label();

    /* Paleta de colores para Fiesta/Beat */
    s_paletteRow = make_settings_row(list, LV_SYMBOL_TINT, 0xFF6B9D,
                                     "Paleta musical", &s_paletteVal, 36, true, true);
    ui_config_refresh_palette_label();

#if ENABLE_SPEAKER
    s_speakerRow = make_settings_row(list, LV_SYMBOL_VOLUME_MAX, 0x00CCCC,
                                     "Probar altavoz", NULL, 36, false, true);
    lv_obj_t *spkVal = lv_obj_get_child(s_speakerRow, 2);
    if (spkVal) {
        lv_label_set_text(spkVal, "Tocar");
    }
#endif

    /* Modo noche */
    lv_obj_t *nightRow = lv_obj_create(list);
    lv_obj_set_width(nightRow, kScreenWidth - 16);
    lv_obj_set_height(nightRow, 36);
    ui_style_card(nightRow);
    lv_obj_clear_flag(nightRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(nightRow, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *nightIco = lv_label_create(nightRow);
    lv_label_set_text(nightIco, LV_SYMBOL_EYE_CLOSE);
    lv_obj_set_style_text_color(nightIco, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align(nightIco, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *nightLbl = lv_label_create(nightRow);
    lv_label_set_text(nightLbl, "Modo noche");
    lv_obj_set_style_text_color(nightLbl, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align(nightLbl, LV_ALIGN_LEFT_MID, 22, 0);

    lv_obj_t *nightVal = lv_label_create(nightRow);
    lv_label_set_text(nightVal, "22:00 - 07:00");
    lv_obj_set_style_text_color(nightVal, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align(nightVal, LV_ALIGN_RIGHT_MID, -40, 0);

    lv_obj_t *nightSw = lv_switch_create(nightRow);
    lv_obj_set_size(nightSw, 40, 22);
    ui_style_switch(nightSw);
    lv_obj_align(nightSw, LV_ALIGN_RIGHT_MID, 0, 0);
    s_nightSw = nightSw;
    ui_config_sync_night_switch(ui_settings_get_night_mode());

    /* Informacion */
    lv_obj_t *infoRow = make_settings_row(list, "i", UI_COLOR_TEXT_DIM,
                                          "Informacion", NULL, 36, true, false);
    lv_obj_t *infoVal = lv_obj_get_child(infoRow, 2);
    if (infoVal) {
        lv_label_set_text(infoVal, FW_VERSION);
    }

#if ENABLE_RAINMAKER
    s_configStatusLbl = lv_label_create(list);
    lv_label_set_text(s_configStatusLbl, "");
    lv_obj_set_style_text_color(s_configStatusLbl, lv_color_hex(0xFFAA44), LV_PART_MAIN);
    lv_obj_set_width(s_configStatusLbl, kScreenWidth - 16);
    lv_obj_add_flag(s_configStatusLbl, LV_OBJ_FLAG_HIDDEN);
#endif

    ui_BtnConfigBack = NULL;
    ui_Config = tab;
    return tab;
}

void ui_config_set_wifi_state(ui_wifi_state_t state, const char *text)
{
    if (!s_wifiLbl) {
        return;
    }

    if (state == UI_WIFI_CONNECTED) {
        if (text && text[0] != '\0') {
            lv_label_set_text(s_wifiLbl, text);
        } else {
            lv_label_set_text(s_wifiLbl, "Conectado");
        }
        lv_obj_set_style_text_color(s_wifiLbl, lv_color_hex(UI_COLOR_OK), LV_PART_MAIN);
        if (s_wifiIcon) {
            lv_obj_set_style_text_color(s_wifiIcon, lv_color_hex(UI_COLOR_OK), LV_PART_MAIN);
        }
    } else if (state == UI_WIFI_CONNECTING) {
        lv_label_set_text(s_wifiLbl, (text && text[0] != '\0') ? text : "Conectando...");
        lv_obj_set_style_text_color(s_wifiLbl, lv_color_hex(0x4488FF), LV_PART_MAIN);
        if (s_wifiIcon) {
            lv_obj_set_style_text_color(s_wifiIcon, lv_color_hex(0x4488FF), LV_PART_MAIN);
        }
    } else {
        lv_label_set_text(s_wifiLbl, (text && text[0] != '\0') ? text : "Sin conexion");
        lv_obj_set_style_text_color(s_wifiLbl, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
        if (s_wifiIcon) {
            lv_obj_set_style_text_color(s_wifiIcon, lv_color_hex(0x666677), LV_PART_MAIN);
        }
    }
}

void ui_config_set_wifi_text(const char *text, bool online)
{
    ui_config_set_wifi_state(online ? UI_WIFI_CONNECTED : UI_WIFI_DISCONNECTED, text);
}

void ui_config_set_wifi_status(bool online)
{
    ui_config_set_wifi_text(online ? NULL : "Sin conexion", online);
}

lv_obj_t *ui_config_get_brightness_slider(void)
{
    return s_settingsBrilloSlider;
}

lv_obj_t *ui_config_get_mic_sensitivity_slider(void)
{
    return s_micSensSlider;
}

void ui_config_set_status(const char *text, uint32_t color_hex)
{
    if (!s_configStatusLbl || !text) {
        return;
    }
    if (text[0] == '\0') {
        lv_obj_add_flag(s_configStatusLbl, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_label_set_text(s_configStatusLbl, text);
    lv_obj_set_style_text_color(s_configStatusLbl, lv_color_hex(color_hex), LV_PART_MAIN);
    lv_obj_clear_flag(s_configStatusLbl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(s_configStatusLbl);
}

void ui_config_set_mic_sensitivity_pct(uint8_t pct)
{
    if (!s_micSensPct) {
        return;
    }
    char buf[8];
    snprintf(buf, sizeof(buf), "%u%%", pct);
    lv_label_set_text(s_micSensPct, buf);
}

void ui_config_update_mic_sensitivity_meter(int sound_level, int music_level)
{
    if (s_micSensLevelLbl) {
        char buf[16];
        snprintf(buf, sizeof(buf), "L %d", sound_level);
        lv_label_set_text(s_micSensLevelLbl, buf);
    }
    if (s_micSensBar) {
        int barVal = music_level;
        if (barVal > MIC_BAR_MAX) {
            barVal = MIC_BAR_MAX;
        }
        if (barVal < 0) {
            barVal = 0;
        }
        lv_bar_set_value(s_micSensBar, barVal, LV_ANIM_OFF);
    }
}

lv_obj_t *ui_config_get_timer_row(void)
{
    return s_timerRow;
}

lv_obj_t *ui_config_get_sunrise_row(void)
{
    return s_sunriseRow;
}

lv_obj_t *ui_config_get_palette_row(void)
{
    return s_paletteRow;
}

void ui_config_refresh_sunrise_label(void)
{
    if (!s_sunriseVal) {
        return;
    }
    char buf[12];
    ui_settings_format_sunrise_label(buf, sizeof(buf));
    lv_label_set_text(s_sunriseVal, buf);
}

void ui_config_refresh_palette_label(void)
{
    if (!s_paletteVal) {
        return;
    }
    lv_label_set_text(s_paletteVal, music_effects_palette_name(music_effects_get_palette()));
}

lv_obj_t *ui_config_get_speaker_row(void)
{
    return s_speakerRow;
}

lv_obj_t *ui_config_get_night_switch(void)
{
    return s_nightSw;
}

void ui_config_refresh_timer_label(void)
{
    if (!s_timerVal) {
        return;
    }
    char buf[20];
    ui_settings_format_timer_label(buf, sizeof(buf));
    lv_label_set_text(s_timerVal, buf);
}

void ui_config_sync_night_switch(bool enabled)
{
    if (!s_nightSw) {
        return;
    }
    if (enabled) {
        lv_obj_add_state(s_nightSw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(s_nightSw, LV_STATE_CHECKED);
    }
}
