#ifndef LAMPARA_UI_H
#define LAMPARA_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern lv_obj_t *ui_Control;
extern lv_obj_t *ui_Config;
extern lv_obj_t *ui_Prov;
extern lv_obj_t *ui_ColorWheel;
extern lv_obj_t *ui_SliderBrillo;
extern lv_obj_t *ui_DropdownEfectos;
extern lv_obj_t *ui_SliderVelocidad;
extern lv_obj_t *ui_SwitchFiesta;
extern lv_obj_t *ui_SwitchPower;
extern lv_obj_t *ui_LabelEstado;
extern lv_obj_t *ui_LabelClock;
extern lv_obj_t *ui_BtnConfigBack;
extern lv_obj_t *ui_BtnStartProv;
extern lv_obj_t *ui_BtnProvExit;
extern lv_obj_t *ui_MicSensSlider;
extern lv_obj_t *ui_MicSensBar;

void ui_init(void);
void ui_show_control_screen(void);
void ui_show_config_screen(void);
void ui_show_prov_screen(void);
void ui_set_active_tab(uint8_t tab);
bool ui_is_settings_tab(void);

#ifdef __cplusplus
}
#endif

#endif
