#include "ui_control_screen.h"
#include "ui_config_screen.h"
#include "ui_effects.h"
#include "lampara_ui.h"
#include "ui_theme.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdint.h>

#define UI_TAB_COLOR     0
#define UI_TAB_EFFECTS   1
#define UI_TAB_SETTINGS  2
#define PRESET_COUNT     7

static lv_obj_t *s_header = NULL;
static lv_obj_t *s_headerTitle = NULL;
static lv_obj_t *s_wifiIcon = NULL;
static lv_obj_t *s_tabColor = NULL;
static lv_obj_t *s_tabEffects = NULL;
static lv_obj_t *s_tabSettings = NULL;
static lv_obj_t *s_navBar = NULL;
static lv_obj_t *s_navBtns[3];
static lv_obj_t *s_navIndicators[3];
static lv_obj_t *s_effectList = NULL;
static lv_obj_t *s_btnDirLeft = NULL;
static lv_obj_t *s_btnDirRight = NULL;
static lv_obj_t *s_lblBrilloPct = NULL;
static lv_obj_t *s_lblVelPct = NULL;
static lv_obj_t *s_lblSettingsBrillo = NULL;
static lv_obj_t *s_colorCenter = NULL;
static lv_obj_t *s_presetRow = NULL;
static lv_obj_t *s_presetBtns[PRESET_COUNT];
static uint8_t s_activeTab = UI_TAB_COLOR;
static bool s_effectsBuilt = false;
static bool s_settingsBuilt = false;
static lv_event_cb_t s_cbEffect = NULL;
static lv_event_cb_t s_cbPreset = NULL;
static lv_event_cb_t s_cbNav = NULL;
static lv_event_cb_t s_cbSpeed = NULL;
static lv_event_cb_t s_cbDir = NULL;
static void (*s_cbSettingsBind)(void) = NULL;

static const struct {
    const char *title;
    const char *icon;
} kTabInfo[] = {
    { "LAMPARA V3", LV_SYMBOL_TINT },
    { "EFECTOS",    LV_SYMBOL_SHUFFLE },
    { "AJUSTES",    LV_SYMBOL_SETTINGS },
};

static const struct {
    uint16_t hue;
    uint8_t sat;
    const char *label;
} kPresets[] = {
    { 200, 25, "Frio" },
    { 0,   0,  "Neutro" },
    { 35,  55, "Calido" },
    { 0,   255, "R" },
    { 85,  255, "G" },
    { 170, 255, "B" },
    { 280, 255, "V" },
};

static lv_color_t ui_color_from_hue_sat(uint16_t hue, uint8_t sat255)
{
    if (sat255 == 0) {
        return lv_color_hex(0xE8E8F0);
    }
    const uint8_t sat100 = (uint8_t)((sat255 * 100U) / 255U);
    return lv_color_hsv_to_rgb(hue, sat100, 100);
}

static void style_color_dot(lv_obj_t *dot, uint16_t hue, uint8_t sat)
{
    lv_obj_set_style_bg_color(dot, ui_color_from_hue_sat(hue, sat), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
}

static lv_obj_t *make_preset_btn(lv_obj_t *parent, int idx)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_height(btn, 44);
    lv_obj_set_flex_grow(btn, 1);
    lv_obj_set_style_radius(btn, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, lv_color_hex(UI_COLOR_CARD_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);

    lv_obj_t *dot = lv_obj_create(btn);
    lv_obj_set_size(dot, 18, 18);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(dot, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(dot, lv_color_hex(0x555566), LV_PART_MAIN);
    lv_obj_align(dot, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
    style_color_dot(dot, kPresets[idx].hue, kPresets[idx].sat);

    if (kPresets[idx].label[0]) {
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, kPresets[idx].label);
        lv_obj_set_style_text_color(lbl, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
        lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -2);
    }

    s_presetBtns[idx] = btn;
    return btn;
}

static lv_obj_t *make_toggle_panel(lv_obj_t *parent, const char *icon, const char *text,
                                   lv_obj_t **outSwitch, int w)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, w, 54);
    ui_style_card(card);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_pad_all(card, 4, LV_PART_MAIN);

    lv_obj_t *ico = lv_label_create(card);
    lv_label_set_text(ico, icon);
    lv_obj_set_style_text_color(ico, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align(ico, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *lbl = lv_label_create(card);
    lv_label_set_text(lbl, text);
    lv_obj_set_width(lbl, w - 22);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_anim_speed(lbl, 18, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align_to(lbl, ico, LV_ALIGN_OUT_RIGHT_MID, 2, 0);

    *outSwitch = lv_switch_create(card);
    lv_obj_set_size(*outSwitch, 42, 22);
    ui_style_switch(*outSwitch);
    lv_obj_align(*outSwitch, LV_ALIGN_BOTTOM_MID, 0, 0);
    return card;
}

static void build_header(lv_obj_t *parent)
{
    s_header = lv_obj_create(parent);
    lv_obj_set_size(s_header, kScreenWidth, UI_HDR_H);
    lv_obj_align(s_header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(s_header, lv_color_hex(0x14141A), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_header, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(s_header, 8, LV_PART_MAIN);
    lv_obj_clear_flag(s_header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *ico = lv_label_create(s_header);
    lv_label_set_text(ico, LV_SYMBOL_TINT);
    lv_obj_set_style_text_color(ico, lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_align(ico, LV_ALIGN_LEFT_MID, 0, 0);

    s_headerTitle = lv_label_create(s_header);
    lv_label_set_text(s_headerTitle, kTabInfo[0].title);
    lv_obj_set_style_text_color(s_headerTitle, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align_to(s_headerTitle, ico, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    ui_BtnConfig = lv_btn_create(s_header);
    lv_obj_set_size(ui_BtnConfig, 28, 24);
    lv_obj_align(ui_BtnConfig, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_opa(ui_BtnConfig, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(ui_BtnConfig, 0, LV_PART_MAIN);
    lv_obj_t *gear = lv_label_create(ui_BtnConfig);
    lv_label_set_text(gear, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(gear, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_center(gear);

    s_wifiIcon = lv_label_create(s_header);
    lv_label_set_text(s_wifiIcon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(s_wifiIcon, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_align_to(s_wifiIcon, ui_BtnConfig, LV_ALIGN_OUT_LEFT_MID, -6, 0);

    ui_LabelEstado = lv_label_create(s_header);
    lv_label_set_text(ui_LabelEstado, "");
    lv_obj_set_style_text_color(ui_LabelEstado, lv_color_hex(UI_COLOR_OK), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_LabelEstado, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align_to(ui_LabelEstado, s_wifiIcon, LV_ALIGN_OUT_LEFT_MID, -4, 0);
    lv_obj_add_flag(ui_LabelEstado, LV_OBJ_FLAG_HIDDEN);
}

static void build_color_tab(lv_obj_t *parent)
{
    s_tabColor = lv_obj_create(parent);
    lv_obj_set_size(s_tabColor, kScreenWidth, UI_CONTENT_H);
    lv_obj_align(s_tabColor, LV_ALIGN_TOP_MID, 0, UI_HDR_H);
    ui_style_screen(s_tabColor);

    /* Rueda mas gruesa hacia adentro (arco ancho), selector visible */
    ui_ColorWheel = lv_colorwheel_create(s_tabColor, true);
    lv_obj_set_size(ui_ColorWheel, 112, 112);
    lv_obj_set_pos(ui_ColorWheel, 2, 0);
    lv_obj_set_style_arc_width(ui_ColorWheel, 18, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ui_ColorWheel, 18, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(ui_ColorWheel, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_border_width(ui_ColorWheel, 3, LV_PART_KNOB);
    lv_obj_set_style_border_color(ui_ColorWheel, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_radius(ui_ColorWheel, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_pad_all(ui_ColorWheel, 0, LV_PART_KNOB);

    /* Circulo central con el color seleccionado */
    s_colorCenter = lv_obj_create(s_tabColor);
    lv_obj_set_size(s_colorCenter, 34, 34);
    lv_obj_set_style_radius(s_colorCenter, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_colorCenter, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_colorCenter, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align_to(s_colorCenter, ui_ColorWheel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(s_colorCenter, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(s_colorCenter);
    style_color_dot(s_colorCenter, 0, 255);

    lv_obj_t *rightCol = lv_obj_create(s_tabColor);
    lv_obj_set_size(rightCol, 188, UI_COLOR_TOP_H);
    lv_obj_set_pos(rightCol, 128, 0);
    lv_obj_set_style_bg_opa(rightCol, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(rightCol, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(rightCol, 0, LV_PART_MAIN);
    lv_obj_clear_flag(rightCol, LV_OBJ_FLAG_SCROLLABLE);

    /* Panel brillo */
    lv_obj_t *brPanel = lv_obj_create(rightCol);
    lv_obj_set_size(brPanel, 184, 58);
    lv_obj_set_pos(brPanel, 0, 2);
    ui_style_card(brPanel);
    lv_obj_clear_flag(brPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(brPanel, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *sunIco = lv_obj_create(brPanel);
    lv_obj_set_size(sunIco, 14, 14);
    lv_obj_set_style_radius(sunIco, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sunIco, lv_color_hex(0xFFCC33), LV_PART_MAIN);
    lv_obj_set_style_border_width(sunIco, 0, LV_PART_MAIN);
    lv_obj_align(sunIco, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(sunIco, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *lblBr = lv_label_create(brPanel);
    lv_label_set_text(lblBr, "BRILLO");
    lv_obj_set_style_text_color(lblBr, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_align_to(lblBr, sunIco, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    s_lblBrilloPct = lv_label_create(brPanel);
    lv_label_set_text(s_lblBrilloPct, "50%");
    lv_obj_set_style_text_color(s_lblBrilloPct, lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_align(s_lblBrilloPct, LV_ALIGN_TOP_RIGHT, 0, 0);

    ui_SliderBrillo = lv_slider_create(brPanel);
    lv_slider_set_range(ui_SliderBrillo, 0, 255);
    lv_slider_set_value(ui_SliderBrillo, 128, LV_ANIM_OFF);
    lv_obj_set_width(ui_SliderBrillo, 168);
    lv_obj_align(ui_SliderBrillo, LV_ALIGN_BOTTOM_MID, 0, 0);
    ui_style_slider(ui_SliderBrillo);

    /* Fiesta + Encendido: icono arriba, titulo, toggle abajo */
    lv_obj_t *toggleRow = lv_obj_create(rightCol);
    lv_obj_set_size(toggleRow, 184, 56);
    lv_obj_set_pos(toggleRow, 0, 64);
    lv_obj_set_style_bg_opa(toggleRow, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(toggleRow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(toggleRow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(toggleRow, 6, LV_PART_MAIN);
    lv_obj_set_flex_flow(toggleRow, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(toggleRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(toggleRow, LV_OBJ_FLAG_CLICKABLE);

    make_toggle_panel(toggleRow, LV_SYMBOL_REFRESH, "MODO FIESTA", &ui_SwitchFiesta, 89);
    make_toggle_panel(toggleRow, LV_SYMBOL_POWER, "ENCENDIDO", &ui_SwitchPower, 89);
    lv_obj_add_state(ui_SwitchPower, LV_STATE_CHECKED);

    /* Presets a ancho completo en la parte inferior */
    s_presetRow = lv_obj_create(s_tabColor);
    lv_obj_set_size(s_presetRow, kScreenWidth - 8, UI_PRESET_ROW_H);
    lv_obj_align(s_presetRow, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(s_presetRow, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_presetRow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_presetRow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(s_presetRow, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(s_presetRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_presetRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(s_presetRow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_presetRow, LV_OBJ_FLAG_CLICKABLE);

    for (int i = 0; i < PRESET_COUNT; i++) {
        make_preset_btn(s_presetRow, i);
    }
}

static void build_effects_tab(lv_obj_t *parent)
{
    s_tabEffects = lv_obj_create(parent);
    lv_obj_set_size(s_tabEffects, kScreenWidth, UI_CONTENT_H);
    lv_obj_align(s_tabEffects, LV_ALIGN_TOP_MID, 0, UI_HDR_H);
    ui_style_screen(s_tabEffects);
    lv_obj_add_flag(s_tabEffects, LV_OBJ_FLAG_HIDDEN);

    s_effectList = lv_obj_create(s_tabEffects);
    lv_obj_set_size(s_effectList, 118, UI_CONTENT_H - 8);
    lv_obj_set_pos(s_effectList, 4, 4);
    ui_style_card(s_effectList);
    lv_obj_set_style_pad_all(s_effectList, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_row(s_effectList, 3, LV_PART_MAIN);
    lv_obj_set_flex_flow(s_effectList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_effectList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(s_effectList, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_effectList, LV_SCROLLBAR_MODE_AUTO);

    for (uint16_t i = 0; i < UI_EFFECT_OPTION_COUNT; i++) {
        lv_obj_t *btn = lv_btn_create(s_effectList);
        lv_obj_set_width(btn, 104);
        lv_obj_set_height(btn, 24);
        lv_obj_set_style_radius(btn, 6, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(UI_COLOR_ACCENT), LV_STATE_CHECKED);
        lv_obj_set_style_text_color(btn, lv_color_hex(UI_COLOR_TEXT), LV_STATE_CHECKED);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, ui_effect_name_at(i));
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_center(lbl);
    }

    lv_obj_t *rightCol = lv_obj_create(s_tabEffects);
    lv_obj_set_size(rightCol, 186, UI_CONTENT_H - 8);
    lv_obj_set_pos(rightCol, 128, 4);
    lv_obj_set_style_bg_opa(rightCol, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(rightCol, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(rightCol, 0, LV_PART_MAIN);
    lv_obj_clear_flag(rightCol, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lblVel = lv_label_create(rightCol);
    lv_label_set_text(lblVel, LV_SYMBOL_LOOP " VELOCIDAD");
    lv_obj_set_style_text_color(lblVel, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font(lblVel, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(lblVel, 0, 4);

    s_lblVelPct = lv_label_create(rightCol);
    lv_label_set_text(s_lblVelPct, "50%");
    lv_obj_set_style_text_color(s_lblVelPct, lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_align(s_lblVelPct, LV_ALIGN_TOP_RIGHT, 0, 4);

    ui_SliderVelocidad = lv_slider_create(rightCol);
    lv_slider_set_range(ui_SliderVelocidad, 0, 100);
    lv_slider_set_value(ui_SliderVelocidad, 50, LV_ANIM_OFF);
    lv_obj_set_width(ui_SliderVelocidad, 176);
    lv_obj_set_pos(ui_SliderVelocidad, 0, 24);
    ui_style_slider(ui_SliderVelocidad);

    lv_obj_t *lblDir = lv_label_create(rightCol);
    lv_label_set_text(lblDir, LV_SYMBOL_DIRECTORY " DIRECCION");
    lv_obj_set_style_text_color(lblDir, lv_color_hex(UI_COLOR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font(lblDir, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_pos(lblDir, 0, 58);

    s_btnDirLeft = lv_btn_create(rightCol);
    lv_obj_set_size(s_btnDirLeft, 82, 30);
    lv_obj_set_pos(s_btnDirLeft, 0, 78);
    lv_obj_set_style_radius(s_btnDirLeft, 6, LV_PART_MAIN);
    lv_obj_add_flag(s_btnDirLeft, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_t *lL = lv_label_create(s_btnDirLeft);
    lv_label_set_text(lL, LV_SYMBOL_LEFT);
    lv_obj_center(lL);

    s_btnDirRight = lv_btn_create(rightCol);
    lv_obj_set_size(s_btnDirRight, 82, 30);
    lv_obj_set_pos(s_btnDirRight, 90, 78);
    lv_obj_set_style_radius(s_btnDirRight, 6, LV_PART_MAIN);
    lv_obj_add_flag(s_btnDirRight, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_state(s_btnDirRight, LV_STATE_CHECKED);
    lv_obj_t *lR = lv_label_create(s_btnDirRight);
    lv_label_set_text(lR, LV_SYMBOL_RIGHT);
    lv_obj_center(lR);

    ui_DropdownEfectos = NULL;
}

static void build_nav_bar(lv_obj_t *parent)
{
    static const char *kNavLabels[] = { "COLOR", "EFECTOS", "AJUSTES" };
    static const char *kNavIcons[] = { LV_SYMBOL_TINT, LV_SYMBOL_SHUFFLE, LV_SYMBOL_SETTINGS };

    s_navBar = lv_obj_create(parent);
    lv_obj_set_size(s_navBar, kScreenWidth, UI_NAV_H);
    lv_obj_align(s_navBar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(s_navBar, lv_color_hex(0x14141A), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_navBar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_navBar, 0, LV_PART_MAIN);
    lv_obj_clear_flag(s_navBar, LV_OBJ_FLAG_SCROLLABLE);

    const int btnW = kScreenWidth / 3;
    for (int i = 0; i < 3; i++) {
        s_navIndicators[i] = lv_obj_create(s_navBar);
        lv_obj_set_size(s_navIndicators[i], btnW - 20, 2);
        lv_obj_set_pos(s_navIndicators[i], i * btnW + 10, 0);
        lv_obj_set_style_bg_color(s_navIndicators[i], lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN);
        lv_obj_set_style_border_width(s_navIndicators[i], 0, LV_PART_MAIN);
        lv_obj_set_style_radius(s_navIndicators[i], 1, LV_PART_MAIN);
        lv_obj_clear_flag(s_navIndicators[i], LV_OBJ_FLAG_CLICKABLE);
        if (i != 0) {
            lv_obj_add_flag(s_navIndicators[i], LV_OBJ_FLAG_HIDDEN);
        }

        s_navBtns[i] = lv_btn_create(s_navBar);
        lv_obj_set_size(s_navBtns[i], btnW, UI_NAV_H);
        lv_obj_set_pos(s_navBtns[i], i * btnW, 0);
        lv_obj_set_style_bg_opa(s_navBtns[i], LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(s_navBtns[i], 0, LV_PART_MAIN);
        lv_obj_set_style_radius(s_navBtns[i], 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(s_navBtns[i], 0, LV_PART_MAIN);
        lv_obj_set_flex_flow(s_navBtns[i], LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(s_navBtns[i], LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(s_navBtns[i], 4, LV_PART_MAIN);

        lv_obj_t *ico = lv_label_create(s_navBtns[i]);
        lv_label_set_text(ico, kNavIcons[i]);

        lv_obj_t *lbl = lv_label_create(s_navBtns[i]);
        lv_label_set_text(lbl, kNavLabels[i]);
    }
}

static void bind_effects_controls(void)
{
    if (s_cbEffect) {
        ui_bind_effect_callbacks(s_cbEffect);
    }
    if (ui_SliderVelocidad && s_cbSpeed) {
        lv_obj_add_event_cb(ui_SliderVelocidad, s_cbSpeed, LV_EVENT_VALUE_CHANGED, NULL);
    }
    if (s_btnDirLeft && s_cbDir) {
        lv_obj_add_event_cb(s_btnDirLeft, s_cbDir, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(s_btnDirRight, s_cbDir, LV_EVENT_CLICKED, NULL);
    }
}

static void ensure_effects_tab(void)
{
    if (s_effectsBuilt || !ui_Control) {
        return;
    }
    build_effects_tab(ui_Control);
    s_effectsBuilt = true;
    bind_effects_controls();
}

static void ensure_settings_tab(void)
{
    if (s_settingsBuilt || !ui_Control) {
        return;
    }
    s_tabSettings = ui_settings_tab_build(ui_Control);
    s_settingsBuilt = true;
    if (s_cbSettingsBind) {
        s_cbSettingsBind();
    }
}

void ui_control_register_callbacks(lv_event_cb_t effect_cb,
                                   lv_event_cb_t preset_cb,
                                   lv_event_cb_t nav_cb,
                                   lv_event_cb_t speed_cb,
                                   lv_event_cb_t dir_cb,
                                   void (*settings_bind_fn)(void))
{
    s_cbEffect = effect_cb;
    s_cbPreset = preset_cb;
    s_cbNav = nav_cb;
    s_cbSpeed = speed_cb;
    s_cbDir = dir_cb;
    s_cbSettingsBind = settings_bind_fn;
}

static void ui_bind_deferred_callbacks(void)
{
    if (s_cbPreset) {
        ui_bind_preset_callbacks(s_cbPreset);
    }
    if (s_cbNav) {
        ui_bind_nav_callbacks(s_cbNav);
    }
}
void ui_set_active_tab(uint8_t tab)
{
    if (tab == UI_TAB_EFFECTS) {
        ensure_effects_tab();
    } else if (tab == UI_TAB_SETTINGS) {
        ensure_settings_tab();
    }

    if (tab > UI_TAB_SETTINGS) {
        tab = UI_TAB_SETTINGS;
    }
    s_activeTab = tab;

    if (s_tabColor) {
        if (tab == UI_TAB_COLOR) lv_obj_clear_flag(s_tabColor, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_tabColor, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_tabEffects) {
        if (tab == UI_TAB_EFFECTS) lv_obj_clear_flag(s_tabEffects, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_tabEffects, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_tabSettings) {
        if (tab == UI_TAB_SETTINGS) lv_obj_clear_flag(s_tabSettings, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_tabSettings, LV_OBJ_FLAG_HIDDEN);
    }

    if (s_headerTitle) {
        lv_label_set_text(s_headerTitle, kTabInfo[tab].title);
    }

    for (int i = 0; i < 3; i++) {
        if (!s_navBtns[i]) continue;
        lv_obj_t *ico = lv_obj_get_child(s_navBtns[i], 0);
        lv_obj_t *lbl = lv_obj_get_child(s_navBtns[i], 1);
        const bool active = (i == (int)tab);
        const lv_color_t col = lv_color_hex(active ? UI_COLOR_ACCENT : UI_COLOR_TEXT_DIM);
        if (ico) lv_obj_set_style_text_color(ico, col, LV_PART_MAIN);
        if (lbl) lv_obj_set_style_text_color(lbl, col, LV_PART_MAIN);
        if (s_navIndicators[i]) {
            if (active) lv_obj_clear_flag(s_navIndicators[i], LV_OBJ_FLAG_HIDDEN);
            else lv_obj_add_flag(s_navIndicators[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

uint8_t ui_get_active_tab(void)
{
    return s_activeTab;
}

bool ui_is_settings_tab(void)
{
    return s_activeTab == UI_TAB_SETTINGS;
}

void ui_update_brightness_label(uint8_t brightness)
{
    if (!s_lblBrilloPct) return;
    char buf[8];
    snprintf(buf, sizeof(buf), "%u%%", (unsigned)((brightness * 100U) / 255U));
    lv_label_set_text(s_lblBrilloPct, buf);
    if (s_lblSettingsBrillo) {
        lv_label_set_text(s_lblSettingsBrillo, buf);
    }
}

void ui_update_speed_label(uint8_t pct)
{
    if (!s_lblVelPct) return;
    char buf[8];
    snprintf(buf, sizeof(buf), "%u%%", (unsigned)pct);
    lv_label_set_text(s_lblVelPct, buf);
}

static void ui_apply_accent_color(uint16_t hue, uint8_t sat)
{
    const lv_color_t accent = ui_color_from_hue_sat(hue, sat);

    if (ui_SliderBrillo) {
        lv_obj_set_style_bg_color(ui_SliderBrillo, accent, LV_PART_INDICATOR);
    }
    if (s_lblBrilloPct) {
        lv_obj_set_style_text_color(s_lblBrilloPct, accent, LV_PART_MAIN);
    }
    if (ui_SwitchFiesta) {
        lv_obj_set_style_bg_color(ui_SwitchFiesta, accent, LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
    if (ui_SwitchPower) {
        lv_obj_set_style_bg_color(ui_SwitchPower, accent, LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
}

void ui_update_color_preview(uint16_t hue, uint8_t sat)
{
    if (!s_colorCenter) {
        return;
    }
    style_color_dot(s_colorCenter, hue, sat);
    ui_apply_accent_color(hue, sat);
    lv_obj_invalidate(s_colorCenter);
}

void ui_update_wifi_icon(bool online)
{
    if (!s_wifiIcon) return;
    lv_obj_set_style_text_color(s_wifiIcon,
                                lv_color_hex(online ? UI_COLOR_OK : 0x555555),
                                LV_PART_MAIN);
}

void ui_highlight_effect(uint16_t idx)
{
    if (!s_effectList) return;
    const uint32_t n = lv_obj_get_child_cnt(s_effectList);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(s_effectList, i);
        if (!btn) continue;
        if (i == idx) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(btn, LV_STATE_CHECKED);
        }
    }
}

void ui_update_direction_buttons(bool reverse)
{
    if (!s_btnDirLeft || !s_btnDirRight) return;
    if (reverse) {
        lv_obj_add_state(s_btnDirLeft, LV_STATE_CHECKED);
        lv_obj_clear_state(s_btnDirRight, LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(s_btnDirLeft, lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(s_btnDirRight, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    } else {
        lv_obj_clear_state(s_btnDirLeft, LV_STATE_CHECKED);
        lv_obj_add_state(s_btnDirRight, LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(s_btnDirRight, lv_color_hex(UI_COLOR_ACCENT), LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(s_btnDirLeft, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    }
}

lv_obj_t *ui_get_effect_list(void)
{
    return s_effectList;
}

lv_obj_t *ui_get_color_tab(void)
{
    return s_tabColor;
}

lv_obj_t *ui_get_dir_btn_left(void)
{
    return s_btnDirLeft;
}

lv_obj_t *ui_get_dir_btn_right(void)
{
    return s_btnDirRight;
}

void ui_set_settings_brightness_label(lv_obj_t *lbl)
{
    s_lblSettingsBrillo = lbl;
}

void ui_bind_nav_callbacks(lv_event_cb_t cb)
{
    for (int i = 0; i < 3; i++) {
        if (s_navBtns[i] && cb) {
            lv_obj_add_event_cb(s_navBtns[i], cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        }
    }
}

void ui_bind_preset_callbacks(lv_event_cb_t cb)
{
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (s_presetBtns[i] && cb) {
            lv_obj_add_event_cb(s_presetBtns[i], cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        }
    }
}

void ui_bind_effect_callbacks(lv_event_cb_t cb)
{
    if (!s_effectList || !cb) return;
    const uint32_t n = lv_obj_get_child_cnt(s_effectList);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *btn = lv_obj_get_child(s_effectList, i);
        if (btn) {
            lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        }
    }
}

void ui_Control_screen_init(void)
{
    ui_Control = lv_obj_create(NULL);
    ui_style_screen(ui_Control);

    build_header(ui_Control);
    build_color_tab(ui_Control);
    build_nav_bar(ui_Control);

    ui_bind_deferred_callbacks();

    ui_set_active_tab(UI_TAB_COLOR);
    ui_update_brightness_label(128);
    ui_update_color_preview(0, 255);
}
