#include "lampara_ui.h"
#include "ui_control_screen.h"
#include "ui_prov_screen.h"
#include "config.h"

lv_obj_t *ui_Control;
lv_obj_t *ui_Config;
lv_obj_t *ui_Prov;
lv_obj_t *ui_ColorWheel;
lv_obj_t *ui_SliderBrillo;
lv_obj_t *ui_DropdownEfectos;
lv_obj_t *ui_SliderVelocidad;
lv_obj_t *ui_SwitchFiesta;
lv_obj_t *ui_SwitchPower;
lv_obj_t *ui_LabelEstado;
lv_obj_t *ui_LabelClock;
lv_obj_t *ui_BtnConfigBack;
lv_obj_t *ui_BtnWifiReset;
lv_obj_t *ui_BtnStartProv;
lv_obj_t *ui_BtnProvExit;
lv_obj_t *ui_MicSensSlider;
lv_obj_t *ui_MicSensBar;

void ui_show_control_screen(void)
{
    if (ui_Control) {
        lv_disp_load_scr(ui_Control);
        lv_obj_invalidate(ui_Control);
    }
}

void ui_show_config_screen(void)
{
    if (ui_Control) {
        lv_disp_load_scr(ui_Control);
        ui_set_active_tab(UI_TAB_SETTINGS);
        lv_obj_invalidate(ui_Control);
    }
}

void ui_show_prov_screen(void)
{
#if RM_PROV_UI_NO_LVGL
    /* Pantalla prov dibujada en ui_prov_raw (TFT directo). */
    return;
#else
    if (!ui_Prov) {
        ui_Prov_screen_init();
    }
    if (ui_Prov) {
        lv_disp_load_scr(ui_Prov);
        lv_obj_invalidate(ui_Prov);
    }
#endif
}

void ui_init(void)
{
    ui_Control_screen_init();
    ui_show_control_screen();
}
