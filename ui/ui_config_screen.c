#include "ui_config_screen.h"
#include "ui_control_screen.h"
#include "ui_theme.h"
#include "lampara_ui.h"
#include "config.h"
#include "led_layout.h"
#include "lvgl.h"
#include <stdio.h>

#define MIC_BAR_MAX 800
#define FW_VERSION  "Lampara V3.0.1"

static lv_obj_t *s_configStatusLbl = NULL;
static lv_obj_t *s_micTestBtnLbl = NULL;
static lv_obj_t *s_ledCalibBtnLbl = NULL;
static lv_obj_t *s_wifiLbl = NULL;
static lv_obj_t *s_settingsBrilloSlider = NULL;
static lv_obj_t *s_settingsBrilloPct = NULL;

static lv_obj_t *make_settings_row(lv_obj_t *parent, const char *title, lv_obj_t **valueOut, int h)
{
    lv_obj_t *row = lv_btn_create(parent);
    lv_obj_set_width(row, kScreenWidth - 16);
    lv_obj_set_height(row, h);
    lv_obj_set_style_bg_color(row, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_radius(row, 8, LV_PART_MAIN);
    lv_obj_set_style_border_color(row, lv_color_hex(UI_COLOR_CARD_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(row, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(row, 10, LV_PART_MAIN);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_color(lbl, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *val = lv_label_create(row);
    lv_label_set_text(val, "");
    lv_obj_set_style_text_color(val, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align(val, LV_ALIGN_RIGHT_MID, -14, 0);
  if (valueOut) {
        *valueOut = val;
    }

    lv_obj_t *chev = lv_label_create(row);
    lv_label_set_text(chev, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(chev, lv_color_hex(0x555566), LV_PART_MAIN);
    lv_obj_align(chev, LV_ALIGN_RIGHT_MID, 0, 0);

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
    lv_obj_set_style_pad_all(list, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(list, 5, LV_PART_MAIN);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_t *wifiRow = make_settings_row(list, LV_SYMBOL_WIFI " WiFi", &s_wifiLbl, 34);
    lv_label_set_text(s_wifiLbl, "Sin conexion");
    lv_obj_set_style_text_color(s_wifiLbl, lv_color_hex(0xFFAA44), LV_PART_MAIN);
#if ENABLE_RAINMAKER
    ui_BtnStartProv = wifiRow;
#else
    lv_obj_add_state(wifiRow, LV_STATE_DISABLED);
#endif

    lv_obj_t *brRow = lv_obj_create(list);
    lv_obj_set_width(brRow, kScreenWidth - 16);
    lv_obj_set_height(brRow, 52);
    ui_style_card(brRow);
    lv_obj_clear_flag(brRow, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *brTitle = lv_label_create(brRow);
    lv_label_set_text(brTitle, "Brillo global");
    lv_obj_set_style_text_color(brTitle, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align(brTitle, LV_ALIGN_TOP_LEFT, 0, 0);

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

    lv_obj_t *timerRow = make_settings_row(list, LV_SYMBOL_BELL " Temporizador", NULL, 34);
    lv_obj_t *timerVal = lv_obj_get_child(timerRow, 1);
    if (timerVal) {
        lv_label_set_text(timerVal, "Desactivado");
    }
    lv_obj_add_state(timerRow, LV_STATE_DISABLED);

    lv_obj_t *nightRow = make_settings_row(list, LV_SYMBOL_EYE_CLOSE " Modo noche", NULL, 34);
    lv_obj_t *nightVal = lv_obj_get_child(nightRow, 1);
    if (nightVal) {
        lv_label_set_text(nightVal, "22:00-07:00");
    }
    lv_obj_add_state(nightRow, LV_STATE_DISABLED);

    ui_BtnMicTest = lv_btn_create(list);
    lv_obj_set_width(ui_BtnMicTest, kScreenWidth - 16);
    lv_obj_set_height(ui_BtnMicTest, 34);
    lv_obj_set_style_bg_color(ui_BtnMicTest, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_radius(ui_BtnMicTest, 8, LV_PART_MAIN);
    s_micTestBtnLbl = lv_label_create(ui_BtnMicTest);
    lv_label_set_text(s_micTestBtnLbl, LV_SYMBOL_AUDIO " Probar microfono");
    lv_obj_center(s_micTestBtnLbl);

    ui_LabelMicTest = lv_label_create(list);
    lv_label_set_text(ui_LabelMicTest, "Apagado");
    lv_obj_set_style_text_color(ui_LabelMicTest, lv_color_hex(0x888888), LV_PART_MAIN);

    ui_BarMicTest = lv_bar_create(list);
    lv_bar_set_range(ui_BarMicTest, 0, MIC_BAR_MAX);
    lv_bar_set_value(ui_BarMicTest, 0, LV_ANIM_OFF);
    lv_obj_set_size(ui_BarMicTest, kScreenWidth - 16, 12);
    lv_obj_set_style_bg_color(ui_BarMicTest, lv_color_hex(0x222233), LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_BarMicTest, lv_color_hex(0x00CC88), LV_PART_INDICATOR);
    lv_obj_add_flag(ui_BarMicTest, LV_OBJ_FLAG_HIDDEN);

    ui_BtnLedCalib = lv_btn_create(list);
    lv_obj_set_width(ui_BtnLedCalib, kScreenWidth - 16);
    lv_obj_set_height(ui_BtnLedCalib, 34);
    lv_obj_set_style_bg_color(ui_BtnLedCalib, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_radius(ui_BtnLedCalib, 8, LV_PART_MAIN);
    s_ledCalibBtnLbl = lv_label_create(ui_BtnLedCalib);
    lv_label_set_text(s_ledCalibBtnLbl, LV_SYMBOL_EYE_OPEN " Calibrar LEDs");
    lv_obj_center(s_ledCalibBtnLbl);

    ui_LabelLedCalib = lv_label_create(list);
    lv_label_set_text(ui_LabelLedCalib, "Apagado");
    lv_obj_set_style_text_color(ui_LabelLedCalib, lv_color_hex(0x888888), LV_PART_MAIN);

    lv_obj_t *infoRow = make_settings_row(list, LV_SYMBOL_LIST " Informacion", NULL, 34);
    lv_obj_t *infoVal = lv_obj_get_child(infoRow, 1);
    if (infoVal) {
        lv_label_set_text(infoVal, FW_VERSION);
        lv_obj_set_style_text_color(infoVal, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    }
    lv_obj_add_state(infoRow, LV_STATE_DISABLED);

#if ENABLE_RAINMAKER
    s_configStatusLbl = lv_label_create(list);
    lv_label_set_text(s_configStatusLbl, "");
    lv_obj_set_style_text_color(s_configStatusLbl, lv_color_hex(0xFFAA44), LV_PART_MAIN);
    lv_obj_set_width(s_configStatusLbl, kScreenWidth - 16);
#else
    lv_obj_t *off = lv_label_create(list);
    lv_label_set_text(off, "RainMaker desactivado");
    lv_obj_set_style_text_color(off, lv_color_hex(0x888888), LV_PART_MAIN);
#endif

    ui_BtnConfigBack = NULL;
    ui_Config = tab;
    return tab;
}

void ui_Config_screen_init(void)
{
    /* Compatibilidad: la pestaña AJUSTES se crea en ui_Control_screen_init */
}

void ui_config_set_wifi_status(bool online)
{
    if (!s_wifiLbl) return;
    if (online) {
        lv_label_set_text(s_wifiLbl, "Conectado");
        lv_obj_set_style_text_color(s_wifiLbl, lv_color_hex(UI_COLOR_OK), LV_PART_MAIN);
    } else {
        lv_label_set_text(s_wifiLbl, "Sin conexion");
        lv_obj_set_style_text_color(s_wifiLbl, lv_color_hex(0xFFAA44), LV_PART_MAIN);
    }
}

lv_obj_t *ui_config_get_brightness_slider(void)
{
    return s_settingsBrilloSlider;
}

void ui_config_set_status(const char *text, uint32_t color_hex)
{
    if (!s_configStatusLbl || !text) {
        return;
    }
    lv_label_set_text(s_configStatusLbl, text);
    lv_obj_set_style_text_color(s_configStatusLbl, lv_color_hex(color_hex), LV_PART_MAIN);
    lv_obj_invalidate(s_configStatusLbl);
}

void ui_config_set_mic_test_active(bool active)
{
    if (!ui_BtnMicTest || !s_micTestBtnLbl) {
        return;
    }
    if (active) {
        lv_label_set_text(s_micTestBtnLbl, LV_SYMBOL_STOP " Microfono ON");
        lv_obj_set_style_bg_color(ui_BtnMicTest, lv_color_hex(UI_COLOR_ACCENT_DIM), LV_PART_MAIN);
        if (ui_LabelMicTest) {
            lv_label_set_text(ui_LabelMicTest, "L 0");
            lv_obj_set_style_text_color(ui_LabelMicTest, lv_color_hex(UI_COLOR_OK), LV_PART_MAIN);
        }
        if (ui_BarMicTest) {
            lv_bar_set_value(ui_BarMicTest, 0, LV_ANIM_OFF);
            lv_obj_clear_flag(ui_BarMicTest, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        lv_label_set_text(s_micTestBtnLbl, LV_SYMBOL_AUDIO " Probar microfono");
        lv_obj_set_style_bg_color(ui_BtnMicTest, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
        if (ui_LabelMicTest) {
            lv_label_set_text(ui_LabelMicTest, "Apagado");
            lv_obj_set_style_text_color(ui_LabelMicTest, lv_color_hex(0x888888), LV_PART_MAIN);
        }
        if (ui_BarMicTest) {
            lv_bar_set_value(ui_BarMicTest, 0, LV_ANIM_OFF);
            lv_obj_add_flag(ui_BarMicTest, LV_OBJ_FLAG_HIDDEN);
        }
    }
    lv_obj_invalidate(ui_BtnMicTest);
}

void ui_config_update_mic_readings(int raw, int level, int span)
{
    (void)raw;
    (void)span;
    if (ui_LabelMicTest) {
        char buf[16];
        snprintf(buf, sizeof(buf), "L %d", level);
        lv_label_set_text(ui_LabelMicTest, buf);
        lv_obj_invalidate(ui_LabelMicTest);
    }
    if (ui_BarMicTest) {
        int barVal = level;
        if (barVal > MIC_BAR_MAX) {
            barVal = MIC_BAR_MAX;
        }
        lv_bar_set_value(ui_BarMicTest, barVal, LV_ANIM_ON);
    }
}

void ui_config_set_led_calib_active(bool active)
{
    if (!ui_BtnLedCalib || !s_ledCalibBtnLbl) {
        return;
    }
    if (active) {
        lv_label_set_text(s_ledCalibBtnLbl, LV_SYMBOL_STOP " Calibracion ON");
        lv_obj_set_style_bg_color(ui_BtnLedCalib, lv_color_hex(0x2244AA), LV_PART_MAIN);
        if (ui_LabelLedCalib) {
            lv_obj_set_style_text_color(ui_LabelLedCalib, lv_color_hex(0x88AAFF), LV_PART_MAIN);
        }
    } else {
        lv_label_set_text(s_ledCalibBtnLbl, LV_SYMBOL_EYE_OPEN " Calibrar LEDs");
        lv_obj_set_style_bg_color(ui_BtnLedCalib, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
        if (ui_LabelLedCalib) {
            lv_label_set_text(ui_LabelLedCalib, "Apagado");
            lv_obj_set_style_text_color(ui_LabelLedCalib, lv_color_hex(0x888888), LV_PART_MAIN);
        }
    }
    lv_obj_invalidate(ui_BtnLedCalib);
}

void ui_config_update_led_calib(uint16_t index, uint16_t total)
{
    if (!ui_LabelLedCalib) {
        return;
    }
    char buf[48];
    const uint8_t lvl = led_level_of(index);
    snprintf(buf, sizeof(buf), "LED %u/%u N%u Br+/-",
             (unsigned)(index + 1U), (unsigned)total, (unsigned)(lvl + 1U));
    lv_label_set_text(ui_LabelLedCalib, buf);
    lv_obj_invalidate(ui_LabelLedCalib);
}
