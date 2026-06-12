#ifndef UI_THEME_H
#define UI_THEME_H

#include "display.h"
#include "lvgl.h"

#define UI_HDR_H         26
#define UI_NAV_H         34
#define UI_PRESET_ROW_H  46
#define UI_CONTENT_H     (kScreenHeight - UI_HDR_H - UI_NAV_H)
#define UI_COLOR_TOP_H   (UI_CONTENT_H - UI_PRESET_ROW_H - 2)

/** Reserva izquierda para barra VU (ancho max = UI_VU_EDGE_W_WIDE). */
#define UI_VU_EDGE_W_DEF   8
#define UI_VU_EDGE_W_WIDE  10
#define UI_VU_GUTTER       UI_VU_EDGE_W_WIDE
#define UI_MAIN_X          UI_VU_GUTTER
#define UI_MAIN_W          ((int)kScreenWidth - UI_VU_GUTTER)
/** Panel derecho en pestañas color/efectos (rueda o lista a la izquierda). */
#define UI_RIGHT_COL_X     118
#define UI_RIGHT_COL_W     (UI_MAIN_W - UI_RIGHT_COL_X - 4)
#define UI_RIGHT_PANEL_W   (UI_MAIN_W - 124)

#define UI_COLOR_BG        0x0E0E12
#define UI_COLOR_CARD        0x1A1A22
#define UI_COLOR_CARD_BORDER 0x2A2A35
#define UI_COLOR_ACCENT      0xE53935
#define UI_COLOR_ACCENT_DIM  0x992222
#define UI_COLOR_TEXT        0xFFFFFF
#define UI_COLOR_TEXT_DIM    0x999999
#define UI_COLOR_OK          0x44CC66

static inline void ui_style_screen(lv_obj_t *obj)
{
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
}

static inline void ui_style_card(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_hex(UI_COLOR_CARD_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 6, LV_PART_MAIN);
}

static inline void ui_style_slider(lv_obj_t *slider)
{
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x2A2A35), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(UI_COLOR_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(UI_COLOR_TEXT), LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 4, LV_PART_KNOB);
}

static inline void ui_style_switch(lv_obj_t *sw)
{
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x3A3A45), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, lv_color_hex(UI_COLOR_ACCENT), LV_PART_INDICATOR | LV_STATE_CHECKED);
}

#endif
