#include "ui_app.h"
#include "config.h"
#include "lamp_log.h"
#include "display.h"
#include <esp_heap_caps.h>
#include "touch_cst820.h"
#include "led_controller.h"
#include "led_calib.h"
#include "music_effects.h"
#include "ui_effects.h"
#include "audio_input.h"
#include "led_layout.h"
#include "pins.h"
#include "rainmaker_app.h"
#if ENABLE_RAINMAKER
#include <WiFi.h>
#endif
#include "lampara_ui.h"
#include "ui_config_screen.h"
#include "ui_control_screen.h"
#include "ui_settings.h"
#include "ui_prov_screen.h"
#if RM_PROV_UI_NO_LVGL
#include "ui_prov_raw.h"
#endif
#include "ir_control.h"
#include <lvgl.h>
#include <Arduino.h>
#include <string.h>
#include <time.h>

static lamp_state_t appState;
static bool syncingUi = false;
static bool onConfigScreen = false;
static bool onProvScreen = false;
static volatile bool pendingOpenConfig = false;
static volatile bool pendingCloseConfig = false;

#if ENABLE_RAINMAKER
static bool rainmakerStarted = false;
static uint32_t rainmakerStartMs = 0;
static bool provUiBoot = false;
static bool provBleStarted = false;
static bool provTouchExitLatched = false;

typedef enum {
    PROV_LOAD_IDLE,
    PROV_LOAD_PAINT,
    PROV_LOAD_QR_CREATE,
    PROV_LOAD_QR_PAINT,
    PROV_LOAD_QR_FLUSH,
    PROV_LOAD_BLE,
    PROV_LOAD_DONE
} prov_load_phase_t;

static prov_load_phase_t provLoadPhase = PROV_LOAD_IDLE;
static uint16_t provLoadFrames = 0;
static uint32_t provLoadMs = 0;

static void bind_control_events(void);
static void bind_config_events(void);
static void bind_prov_events(void);
static void prov_load_start(void);
static void prov_load_tick(void);
static void update_prov_status(void);
#endif

static volatile bool s_graphicsPaused = false;
static uint32_t s_graphicsResumeMs = 0;

void ui_app_pause_graphics(bool pause)
{
    s_graphicsPaused = pause;
    if (pause) {
        s_graphicsResumeMs = 0;
        delay(80);
    }
}

void ui_app_hold_graphics_ms(uint32_t ms)
{
    s_graphicsPaused = true;
    s_graphicsResumeMs = millis() + ms;
}

bool ui_app_graphics_paused(void)
{
    return s_graphicsPaused;
}

static void ui_app_graphics_tick(void)
{
    if (s_graphicsResumeMs != 0 && (int32_t)(millis() - s_graphicsResumeMs) >= 0) {
        s_graphicsResumeMs = 0;
        s_graphicsPaused = false;
    }
}

static bool audioStarted = false;
static uint32_t audioStartMs = 0;
static bool ledCalibActive = false;
static uint8_t savedBrightness = 128;
static bool s_ntpStarted = false;

static void set_led_calib_active(bool active);
static void ui_app_ensure_settings_audio(void);
static void ui_app_led_calib_sync_slider(void);
static void ui_app_led_calib_refresh_ui(void);
static void apply_and_report(void);
static void update_status_label(void);
static void ui_app_start_ntp(void);
static void ui_app_update_wifi_display(void);
static void ui_app_update_header_clock(void);
static void on_brightness_changed(lv_event_t *e);

lamp_state_t *ui_app_get_state(void)
{
    return &appState;
}

#if ENABLE_RAINMAKER
static void ui_app_start_ntp(void)
{
    if (s_ntpStarted || WiFi.status() != WL_CONNECTED) {
        return;
    }
    configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    s_ntpStarted = true;
}

static void ui_app_update_wifi_display(void)
{
    ui_wifi_state_t state = UI_WIFI_DISCONNECTED;
    const char *ssid = NULL;

    if (WiFi.status() == WL_CONNECTED) {
        state = UI_WIFI_CONNECTED;
        ssid = WiFi.SSID().c_str();
        ui_config_set_wifi_state(state, ssid);
        ui_app_start_ntp();
    } else if (rainmaker_app_wifi_provisioned()) {
        state = UI_WIFI_CONNECTING;
        ui_config_set_wifi_state(state, "Conectando...");
    } else {
        ui_config_set_wifi_state(UI_WIFI_DISCONNECTED, "Sin conexion");
    }
    ui_update_wifi_status(state, ssid);
}
#endif

static void ui_app_update_header_clock(void)
{
    static uint32_t lastMs = 0;
    const uint32_t now = millis();
    if ((now - lastMs) < 1000U) {
        return;
    }
    lastMs = now;

    struct tm timeinfo;
#if ENABLE_RAINMAKER
    if (WiFi.status() == WL_CONNECTED && getLocalTime(&timeinfo, 0)) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        ui_update_header_clock(buf);
        return;
    }
#endif
    ui_update_header_clock("--:--");
}

static void update_status_label(void)
{
    if (ui_LabelEstado && led_calib_is_active()) {
        char buf[48];
        snprintf(buf, sizeof(buf), "LED %u/%u N%u",
                 (unsigned)(led_calib_get_index() + 1U),
                 (unsigned)led_calib_get_count(),
                 (unsigned)(led_level_of(led_calib_get_index()) + 1U));
        lv_label_set_text(ui_LabelEstado, buf);
        lv_obj_set_style_text_color(ui_LabelEstado, lv_color_hex(0x88AAFF), LV_PART_MAIN);
        lv_obj_clear_flag(ui_LabelEstado, LV_OBJ_FLAG_HIDDEN);
        return;
    }

#if ENABLE_RAINMAKER
    ui_app_update_wifi_display();
#else
    ui_update_wifi_status(UI_WIFI_DISCONNECTED, NULL);
    ui_config_set_wifi_status(false);
#endif
}

#if ENABLE_RAINMAKER
bool ui_app_allow_ble_provision(void)
{
    if (onProvScreen) {
        return provLoadPhase == PROV_LOAD_BLE || provLoadPhase == PROV_LOAD_DONE;
    }
    if (onConfigScreen) return true;
    return true;
}

static void update_prov_status(void)
{
    if (!onProvScreen) return;
    const char *txt = NULL;
    uint32_t color = 0xFFAA44;
    if (rainmaker_app_is_online()) {
        txt = "Conectado - pulsa Salir"; color = 0x00FFAA;
    } else if (rainmaker_app_provisioning_active()) {
#if RM_PROV_UI_SHOW_QR
        txt = "Escanea QR en la app"; color = 0xFFAA44;
#else
        txt = "Conecta desde lista BLE"; color = 0xFFAA44;
#endif
    } else if (provBleStarted) {
        txt = "BLE activo"; color = 0xFFAA44;
    } else if (provLoadPhase == PROV_LOAD_DONE) {
#if RM_PROV_UI_SHOW_QR
        txt = "Listo para escanear"; color = 0xFFAA44;
#else
        txt = "Listo - busca en app BLE"; color = 0xFFAA44;
#endif
    }
    if (!txt) return;
#if RM_PROV_UI_NO_LVGL
    uint16_t c565 = ((color >> 8) & 0xF800) | ((color >> 5) & 0x07E0) | ((color >> 3) & 0x001F);
    ui_prov_raw_set_status(txt, c565);
#else
    ui_prov_set_status(txt, color);
#endif
}

static void prov_load_start(void)
{
    provLoadFrames = 0;
    provLoadMs = millis();
#if RM_PROV_UI_SHOW_QR
    provLoadPhase = PROV_LOAD_PAINT;
#if RM_PROV_UI_NO_LVGL
    ui_prov_raw_set_status("Preparando QR...", 0xFFE0);
#else
    ui_prov_set_status("Preparando...", 0xCCCCCC);
#endif
#else
    /* Sin QR: activar BLE de inmediato (max RAM, prueba lista BLE app) */
    provLoadPhase = PROV_LOAD_BLE;
#if RM_PROV_UI_NO_LVGL
    ui_prov_raw_set_status("Activando BLE...", 0xFFE0);
#else
    ui_prov_set_status("Activando BLE...", 0xFFAA44);
#endif
    LAMP_LOG("PROV: QR off, BLE directo heap=%u\n", ESP.getFreeHeap());
#endif
}

static void prov_phase_ble_tick(void)
{
    if (provBleStarted) {
        return;
    }

    if (provLoadFrames >= 1) {
#if RM_PROV_UI_NO_LVGL
        ui_prov_raw_qr_free();
        ui_prov_raw_set_status("BLE activo - escanea QR", 0x07E0);
#else
#if CONFIG_USE_LVGL_QR
        ui_prov_release_qr_for_ble();
#endif
        ui_prov_set_status("BLE activo - escanea QR", 0xFFAA44);
#endif
        provBleStarted = rainmaker_app_init_done();
        provLoadPhase = PROV_LOAD_DONE;
        LAMP_LOG("PROV UI: RainMaker activo heap=%u\n", ESP.getFreeHeap());
    }
}

static void prov_load_tick(void)
{
    if (!onProvScreen || provLoadPhase == PROV_LOAD_IDLE) return;

    provLoadFrames++;

#if RM_PROV_UI_SHOW_QR && RM_PROV_UI_NO_LVGL
    switch (provLoadPhase) {
        case PROV_LOAD_PAINT:
            if (provLoadFrames >= 2 && (millis() - provLoadMs) >= 100) {
                provLoadPhase = PROV_LOAD_QR_CREATE;
                provLoadFrames = 0;
            }
            break;
        case PROV_LOAD_QR_CREATE:
            if (provLoadFrames >= 1) {
                if (ui_prov_raw_qr_alloc(NULL)) {
                    ui_prov_raw_set_status("Generando QR...", 0xFFE0);
                    provLoadPhase = PROV_LOAD_QR_PAINT;
                } else {
                    ui_prov_raw_set_status("Sin RAM para QR", 0xF800);
                    provBleStarted = rainmaker_app_init_done();
                    provLoadPhase = PROV_LOAD_DONE;
                }
                provLoadFrames = 0;
            }
            break;
        case PROV_LOAD_QR_PAINT:
            if (provLoadFrames >= 1) {
                if (ui_prov_raw_qr_encode_sync()) {
                    ui_prov_raw_qr_paint();
                    ui_prov_raw_qr_free();
                } else {
                    ui_prov_raw_set_status("Error QR", 0xF800);
                    ui_prov_raw_qr_free();
                }
                provBleStarted = rainmaker_app_init_done();
                provLoadPhase = PROV_LOAD_DONE;
                provLoadFrames = 0;
                LAMP_LOG("PROV RAW: QR listo heap=%u\n", ESP.getFreeHeap());
            }
            break;
        default:
            break;
    }
#elif RM_PROV_UI_SHOW_QR
    /* LVGL + QR: fases QR simplificadas; BLE igual */
    switch (provLoadPhase) {
        case PROV_LOAD_PAINT:
            if (provLoadFrames >= 15 && (millis() - provLoadMs) >= 300) {
                provLoadPhase = PROV_LOAD_QR_CREATE;
                provLoadFrames = 0;
                provLoadMs = millis();
            }
            break;
        case PROV_LOAD_QR_CREATE:
            if (provLoadFrames >= 2) {
#if CONFIG_USE_LVGL_QR
                ui_prov_create_qr();
                if (ui_prov_qr_mem_init(NULL)) {
                    ui_prov_qr_encode_reset();
                    ui_prov_qr_encode_async();
                    provLoadPhase = PROV_LOAD_QR_PAINT;
                } else {
                    provLoadPhase = PROV_LOAD_QR_FLUSH;
                }
#else
                provLoadPhase = PROV_LOAD_QR_FLUSH;
#endif
                provLoadFrames = 0;
                provLoadMs = millis();
            }
            break;
        case PROV_LOAD_QR_PAINT:
#if CONFIG_USE_LVGL_QR
            if (!ui_prov_qr_encode_pending() && ui_prov_qr_encode_finished()) {
                if (ui_prov_qr_encode_ok()) {
                    ui_prov_refresh_qr();
                }
                provLoadPhase = PROV_LOAD_QR_FLUSH;
                provLoadFrames = 0;
                provLoadMs = millis();
            }
#endif
            break;
        case PROV_LOAD_QR_FLUSH:
            if (provLoadFrames >= 2 && (millis() - provLoadMs) >= 2000) {
                provLoadPhase = PROV_LOAD_BLE;
                provLoadFrames = 0;
                provLoadMs = millis();
            }
            break;
        case PROV_LOAD_BLE:
            prov_phase_ble_tick();
            break;
        default:
            break;
    }
#else
    /* Sin QR: solo BLE async */
    if (provLoadPhase == PROV_LOAD_BLE) {
        prov_phase_ble_tick();
    }
#endif

    update_prov_status();
}

static void open_config_screen(void)
{
    onConfigScreen = true;
    onProvScreen = false;
    LAMP_LOG_LN("CFG: menu RainMaker");
    ui_show_config_screen();
    bind_config_events();
    ui_app_ensure_settings_audio();
    for (int i = 0; i < 6; i++) {
        lv_timer_handler();
        delay(5);
    }
}

static void close_config_screen(void)
{
    onConfigScreen = false;
    ui_set_active_tab(0);
    update_status_label();
}

static void open_prov_screen(void)
{
    onProvScreen = true;
    onConfigScreen = false;
    provBleStarted = false;
    provLoadPhase = PROV_LOAD_IDLE;
    LAMP_LOG_LN("PROV UI: pantalla RainMaker");
    ui_show_prov_screen();
    bind_prov_events();
    for (int i = 0; i < 8; i++) {
        lv_timer_handler();
        delay(5);
    }
    prov_load_start();
}

static void process_pending_screen(void)
{
    if (pendingCloseConfig) {
        pendingCloseConfig = false;
        close_config_screen();
        return;
    }
    if (pendingOpenConfig) {
        pendingOpenConfig = false;
        open_config_screen();
    }
}

static void ui_app_sync_brightness_sliders(uint8_t value)
{
    syncingUi = true;
    if (ui_SliderBrillo) {
        lv_slider_set_value(ui_SliderBrillo, value, LV_ANIM_OFF);
    }
    lv_obj_t *cfgSlider = ui_config_get_brightness_slider();
    if (cfgSlider) {
        lv_slider_set_value(cfgSlider, value, LV_ANIM_OFF);
    }
    ui_update_brightness_label(value);
    syncingUi = false;
}

static void ui_app_led_calib_sync_slider(void)
{
    if (!ui_SliderBrillo) return;
    syncingUi = true;
    if (ledCalibActive) {
        const int last = (int)led_calib_get_count() - 1;
        const int range = last > 0 ? last : 0;
        lv_slider_set_range(ui_SliderBrillo, 0, range);
        lv_slider_set_value(ui_SliderBrillo, (int)led_calib_get_index(), LV_ANIM_OFF);
        lv_obj_t *cfgSlider = ui_config_get_brightness_slider();
        if (cfgSlider) {
            lv_slider_set_range(cfgSlider, 0, range);
            lv_slider_set_value(cfgSlider, (int)led_calib_get_index(), LV_ANIM_OFF);
        }
    } else {
        lv_slider_set_range(ui_SliderBrillo, 0, 255);
        lv_obj_t *cfgSlider = ui_config_get_brightness_slider();
        if (cfgSlider) {
            lv_slider_set_range(cfgSlider, 0, 255);
        }
        ui_app_sync_brightness_sliders(appState.brightness);
    }
    syncingUi = false;
}

static uint16_t ui_speed_pct_to_hw(uint8_t pct)
{
    const uint8_t inv = (uint8_t)(100U - pct);
    return (uint16_t)(10 + ((uint32_t)inv * 990U) / 100U);
}

static uint8_t ui_speed_hw_to_pct(uint16_t speed)
{
    if (speed <= 10) return 100;
    const uint8_t inv = (uint8_t)(((uint32_t)(speed - 10) * 100U) / 990U);
    return (uint8_t)(100U - inv);
}

static void ui_app_sync_effect_color_bar(void)
{
    const uint16_t idx = ui_effect_dropdown_index(&appState);
    ui_update_effect_color_bar(idx, appState.hue, appState.saturation);
}

static void ui_app_led_calib_refresh_ui(void)
{
    ui_app_led_calib_sync_slider();
    update_status_label();
}

static void ui_app_ensure_settings_audio(void)
{
#if ENABLE_RAINMAKER
    if (onProvScreen || rainmaker_app_provisioning_active()) {
        return;
    }
#endif
    if (audio_input_is_running()) {
        return;
    }
    audioStarted = true;
    audio_input_start();
}

static void set_led_calib_active(bool active)
{
    if (active == ledCalibActive) {
        return;
    }
    ledCalibActive = active;
    if (active) {
        savedBrightness = appState.brightness;
        appState.musicFx = MUSIC_FX_NONE;
        appState.power = true;
        led_calib_set_active(true);
        ui_app_led_calib_refresh_ui();
    } else {
        led_calib_set_active(false);
        appState.brightness = savedBrightness;
        lamp_state_mark_dirty(&appState);
        apply_and_report();
        ui_app_led_calib_sync_slider();
        update_status_label();
    }
}

static void on_mic_sens_changed(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) {
        return;
    }
    const uint8_t pct = (uint8_t)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    audio_input_set_silence_level(audio_input_silence_from_sensitivity_pct(pct));
    ui_config_set_mic_sensitivity_pct(pct);
}

static void on_timer_row_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    ui_settings_timer_cycle();
    ui_config_refresh_timer_label();
}

static void on_night_mode_changed(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) {
        return;
    }
    const bool enabled = lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED);
    ui_settings_set_night_mode(enabled);
    if (ui_settings_service(&appState)) {
        apply_and_report();
        ui_app_sync_from_state();
    }
}

static void on_start_prov(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    LAMP_LOG_LN("CFG: usuario elige RainMaker");
    ui_config_set_status("Reiniciando prov...", 0xFFAA44);
    for (int i = 0; i < 8; i++) {
        lv_timer_handler();
        delay(25);
    }
    rainmaker_app_request_prov_ui_restart();
}

static void on_prov_exit(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    LAMP_LOG_LN("PROV UI: salir");
    rainmaker_app_exit_prov_ui();
}

static void bind_config_events(void)
{
    static bool bound = false;
    if (bound) {
        return;
    }
    bound = true;
#if ENABLE_RAINMAKER
    if (ui_BtnStartProv) {
        lv_obj_add_event_cb(ui_BtnStartProv, on_start_prov, LV_EVENT_CLICKED, NULL);
    }
#endif
    lv_obj_t *cfgSlider = ui_config_get_brightness_slider();
    if (cfgSlider) {
        lv_obj_add_event_cb(cfgSlider, on_brightness_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
    lv_obj_t *micSensSlider = ui_config_get_mic_sensitivity_slider();
    if (micSensSlider) {
        lv_obj_add_event_cb(micSensSlider, on_mic_sens_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
    lv_obj_t *timerRow = ui_config_get_timer_row();
    if (timerRow) {
        lv_obj_add_event_cb(timerRow, on_timer_row_clicked, LV_EVENT_CLICKED, NULL);
    }
    lv_obj_t *nightSw = ui_config_get_night_switch();
    if (nightSw) {
        lv_obj_add_event_cb(nightSw, on_night_mode_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void bind_settings_tab(void)
{
    bind_config_events();
}

static void bind_prov_events(void)
{
    if (ui_BtnProvExit) {
        lv_obj_add_event_cb(ui_BtnProvExit, on_prov_exit, LV_EVENT_CLICKED, NULL);
    }
}

#else /* !ENABLE_RAINMAKER */

bool ui_app_allow_ble_provision(void)
{
    return true;
}

#endif

static void apply_and_report(void)
{
    led_controller_apply(&appState);
    update_status_label();
#if ENABLE_RAINMAKER
    if (rainmakerStarted && rainmaker_app_is_online()) {
        rainmaker_app_report_state(&appState);
    }
#endif
}

void ui_app_ensure_music_audio(void)
{
#if ENABLE_RAINMAKER
    if (onProvScreen || rainmaker_app_provisioning_active()) {
        return;
    }
#endif
    if (!lamp_state_music_active(&appState) || !appState.power) {
        return;
    }
    if (audio_input_is_running()) {
        return;
    }
    audioStarted = true;
    audio_input_start();
}

void ui_app_notify_state_changed(void)
{
    ui_app_ensure_music_audio();
    apply_and_report();
    ui_app_sync_from_state();
}

bool ui_app_ir_allowed(void)
{
#if ENABLE_RAINMAKER
    if (onProvScreen) return false;
    if (rainmaker_app_provisioning_active()) return false;
#endif
    return true;
}

bool ui_app_led_calib_active(void)
{
    return ledCalibActive;
}

void ui_app_led_calib_step(int delta)
{
    if (!ledCalibActive) return;
    if (delta == 0) return;
    led_calib_step(delta > 0 ? 1 : -1);
    ui_app_led_calib_refresh_ui();
}

void ui_app_sync_from_state(void)
{
    if (!ui_ColorWheel) return;
    syncingUi = true;
    if (!ledCalibActive) {
        ui_app_sync_brightness_sliders(appState.brightness);
    }
    if (ui_SwitchPower) {
        if (appState.power) lv_obj_add_state(ui_SwitchPower, LV_STATE_CHECKED);
        else lv_obj_clear_state(ui_SwitchPower, LV_STATE_CHECKED);
    }
    if (ui_SwitchFiesta) {
        if (appState.musicFx == MUSIC_FX_PARTY) lv_obj_add_state(ui_SwitchFiesta, LV_STATE_CHECKED);
        else lv_obj_clear_state(ui_SwitchFiesta, LV_STATE_CHECKED);
    }
    ui_highlight_effect(ui_effect_dropdown_index(&appState));
    if (ui_ColorWheel) {
        lv_color_hsv_t hsv;
        hsv.h = appState.hue;
        hsv.s = (appState.saturation * 100) / 255;
        hsv.v = 100;
        lv_colorwheel_set_hsv(ui_ColorWheel, hsv);
        ui_update_color_preview(appState.hue, appState.saturation);
    }
    if (ui_SliderVelocidad) {
        const uint8_t pct = ui_speed_hw_to_pct(led_controller_get_speed());
        lv_slider_set_value(ui_SliderVelocidad, pct, LV_ANIM_OFF);
        ui_update_speed_label(pct);
    }
    ui_update_direction_buttons(led_controller_get_reverse());
    ui_app_sync_effect_color_bar();
    update_status_label();
    syncingUi = false;
}

static void on_color_changed(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_color_hsv_t hsv = lv_colorwheel_get_hsv(ui_ColorWheel);
    appState.hue = hsv.h;
    appState.saturation = (uint8_t)((hsv.s * 255) / 100);
    ui_update_color_preview(appState.hue, appState.saturation);
    ui_app_sync_effect_color_bar();
    lamp_state_mark_dirty(&appState);
    apply_and_report();
}

static void on_brightness_changed(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    if (led_calib_is_active()) {
        led_calib_set_index((uint16_t)lv_slider_get_value(slider));
        ui_app_led_calib_refresh_ui();
        return;
    }
    appState.brightness = (uint8_t)lv_slider_get_value(slider);
    ui_app_sync_brightness_sliders(appState.brightness);
    lamp_state_mark_dirty(&appState);
    apply_and_report();
}

static void on_effect_btn_clicked(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const uint16_t idx = (uint16_t)(intptr_t)lv_event_get_user_data(e);
    ui_effect_apply_dropdown_index(&appState, idx);
    ui_highlight_effect(idx);
    ui_app_sync_effect_color_bar();
    lamp_state_mark_dirty(&appState);
    ui_app_notify_state_changed();
}

static void on_effect_color_clicked(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    const uint16_t idx = ui_effect_dropdown_index(&appState);
    if (ui_effect_color_mode(idx) == UI_FX_COLOR_RAINBOW) {
        return;
    }

    lv_obj_t *bar = (lv_obj_t *)lv_event_get_target(e);
    lv_indev_t *indev = lv_indev_get_act();
    if (!bar || !indev) {
        return;
    }

    lv_point_t point;
    lv_indev_get_point(indev, &point);
    lv_area_t coords;
    lv_obj_get_coords(bar, &coords);
    const int16_t rel_x = (int16_t)(point.x - coords.x1);
    const int16_t bar_w = (int16_t)lv_obj_get_width(bar);

    appState.hue = ui_effect_hue_from_bar_x(rel_x, bar_w);
    if (appState.saturation == 0) {
        appState.saturation = 255;
    }

    syncingUi = true;
    if (ui_ColorWheel) {
        lv_color_hsv_t hsv;
        hsv.h = appState.hue;
        hsv.s = (uint8_t)((appState.saturation * 100U) / 255U);
        hsv.v = 100;
        lv_colorwheel_set_hsv(ui_ColorWheel, hsv);
    }
    syncingUi = false;

    ui_update_color_preview(appState.hue, appState.saturation);
    ui_app_sync_effect_color_bar();
    lamp_state_mark_dirty(&appState);
    apply_and_report();
}

static void on_speed_changed(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    const uint8_t pct = (uint8_t)lv_slider_get_value(ui_SliderVelocidad);
    ui_update_speed_label(pct);
    led_controller_set_speed(ui_speed_pct_to_hw(pct));
    lamp_state_mark_dirty(&appState);
    apply_and_report();
}

static void on_dir_clicked(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const bool reverse = (lv_event_get_target(e) == ui_get_dir_btn_left());
    led_controller_set_reverse(reverse);
    ui_update_direction_buttons(reverse);
    lamp_state_mark_dirty(&appState);
    apply_and_report();
}

static void on_preset_clicked(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const int idx = (int)(intptr_t)lv_event_get_user_data(e);
    static const struct { uint16_t h; uint8_t s; } presets[7] = {
        { 210, 110 }, { 0, 0 }, { 28, 170 }, { 0, 255 }, { 85, 255 }, { 170, 255 }, { 280, 255 }
    };
    if (idx < 0 || idx >= 7) return;
    appState.hue = presets[idx].h;
    appState.saturation = presets[idx].s;
    syncingUi = true;
    lv_color_hsv_t hsv;
    hsv.h = presets[idx].h;
    hsv.s = (uint8_t)((presets[idx].s * 100U) / 255U);
    hsv.v = 100;
    lv_colorwheel_set_hsv(ui_ColorWheel, hsv);
    syncingUi = false;
    ui_update_color_preview(appState.hue, appState.saturation);
    ui_app_sync_effect_color_bar();
    lamp_state_mark_dirty(&appState);
    apply_and_report();
}

static void on_tab_nav_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const uint8_t tab = (uint8_t)(intptr_t)lv_event_get_user_data(e);
    onConfigScreen = (tab == 2);
    ui_set_active_tab(tab);
    if (tab == 1) {
        ui_highlight_effect(ui_effect_dropdown_index(&appState));
        ui_app_sync_effect_color_bar();
    } else if (tab == 2) {
        ui_app_ensure_settings_audio();
    } else if (!lamp_state_music_active(&appState) && audio_input_is_running()) {
        audio_input_stop();
        audioStarted = false;
    }
}

static void on_party_changed(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    const bool fiesta = lv_obj_has_state(ui_SwitchFiesta, LV_STATE_CHECKED);
    if (fiesta) {
        appState.musicFx = MUSIC_FX_PARTY;
        music_effects_reset();
    } else if (appState.musicFx == MUSIC_FX_PARTY) {
        appState.musicFx = MUSIC_FX_NONE;
    }
    lamp_state_mark_dirty(&appState);
    ui_app_notify_state_changed();
}

static void on_power_changed(lv_event_t *e)
{
    if (syncingUi || lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
    appState.power = lv_obj_has_state(sw, LV_STATE_CHECKED);
    lamp_state_mark_dirty(&appState);
    apply_and_report();
}

static void bind_control_events(void)
{
    if (!ui_ColorWheel) return;
    lv_obj_add_event_cb(ui_ColorWheel, on_color_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_SliderBrillo, on_brightness_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_SwitchFiesta, on_party_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_SwitchPower, on_power_changed, LV_EVENT_VALUE_CHANGED, NULL);
}

void ui_app_setup(void)
{
    lamp_state_init(&appState);

#if ENABLE_RAINMAKER
    provUiBoot = rainmaker_app_consume_prov_ui_boot();
    rainmaker_app_setup(&appState);
    LAMP_LOG("RM setup heap=%u\n", ESP.getFreeHeap());
    Serial.flush();
    rainmakerStarted = true;
    rainmakerStartMs = millis();

    if (provUiBoot) {
        display_init_tft_only();
        LAMP_LOG("PROV RAW TFT heap=%u\n", ESP.getFreeHeap());
        touch_cst820_init();
        ui_prov_raw_init();
        onProvScreen = true;
        provBleStarted = rainmaker_app_init_done();
        prov_load_start();
        return;
    }
#endif

    lv_init();
    display_init();
    LAMP_LOG("Display OK heap=%u\n", ESP.getFreeHeap());
    Serial.flush();
    touch_cst820_init();
    touch_cst820_register_indev();

    ui_control_register_effect_color_cb(on_effect_color_clicked);
    ui_control_register_callbacks(on_effect_btn_clicked,
                                  on_preset_clicked,
                                  on_tab_nav_clicked,
                                  on_speed_changed,
                                  on_dir_clicked,
                                  bind_settings_tab);
    ui_init();
    LAMP_LOG("UI init heap=%u\n", ESP.getFreeHeap());
    Serial.flush();
    bind_control_events();
    ui_app_sync_from_state();

    audio_input_init();
    ui_settings_init();

    for (int i = 0; i < 8; i++) {
        lv_timer_handler();
        delay(10);
    }
    LAMP_LOG("UI lista heap=%u\n", ESP.getFreeHeap());
    Serial.flush();

    led_controller_init();
    LAMP_LOG("LED OK heap=%u\n", ESP.getFreeHeap());
    Serial.flush();

    apply_and_report();
#if ENABLE_RAINMAKER
    rainmaker_app_on_ui_ready();
#endif
#if !ENABLE_RAINMAKER
    audioStartMs = millis() + AUDIO_START_DELAY_MS;
#endif
}

void ui_app_loop(void)
{
#if ENABLE_RAINMAKER
    if (provUiBoot) {
        prov_load_tick();
        {
            char st[48];
            uint32_t color = 0;
            if (rainmaker_app_consume_prov_status(st, sizeof(st), &color)) {
                const uint16_t c565 =
                    (uint16_t)(((color >> 8) & 0xF800) | ((color >> 5) & 0x07E0) | ((color >> 3) & 0x001F));
                ui_prov_raw_set_status(st, c565);
            }
        }
        if (ui_prov_raw_check_exit_pressed()) {
            LAMP_LOG_LN("PROV RAW: Salir -> reinicio UI");
            rainmaker_app_exit_prov_ui();
        }
        rainmaker_app_loop();
        delay(5);
        return;
    }

#if !RM_PROV_UI_NO_LVGL
    process_pending_screen();
#else
    process_pending_screen();
#endif
    ui_app_graphics_tick();
    if (onProvScreen) {
        prov_load_tick();
        {
            char st[48];
            uint32_t color = 0;
            if (rainmaker_app_consume_prov_status(st, sizeof(st), &color)) {
#if RM_PROV_UI_NO_LVGL
                uint16_t c565 = ((color >> 8) & 0xF800) | ((color >> 5) & 0x07E0) | ((color >> 3) & 0x001F);
                ui_prov_raw_set_status(st, c565);
#else
                if (display_lvgl_suspended()) {
                    display_prov_status_banner(st, (uint16_t)color);
                } else {
                    ui_prov_set_status(st, color);
                }
#endif
            }
        }
#if RM_PROV_UI_NO_LVGL
        if (ui_prov_raw_check_exit_pressed()) {
            LAMP_LOG_LN("PROV RAW: Salir -> reinicio");
            rainmaker_app_exit_prov_ui();
        }
#else
        if (display_lvgl_suspended()) {
            uint16_t tx = 0, ty = 0;
            if (touch_cst820_read_screen(&tx, &ty)) {
                if (touch_cst820_hit_prov_exit(tx, ty)) {
                    if (!provTouchExitLatched) {
                        provTouchExitLatched = true;
                        LAMP_LOG_LN("PROV UI: Salir (touch directo)");
                        rainmaker_app_exit_prov_ui();
                    }
                }
            } else {
                provTouchExitLatched = false;
            }
        } else {
            lv_timer_handler();
        }
#endif
        rainmaker_app_loop();
        delay(5);
        return;
    }

    rainmaker_app_loop();

    if (s_graphicsPaused) {
        delay(10);
        return;
    }
#endif

    lv_timer_handler();

    ui_app_update_header_clock();
    {
        static uint32_t s_lastWifiUiMs = 0;
        const uint32_t nowMs = millis();
        if ((nowMs - s_lastWifiUiMs) >= 400U) {
            s_lastWifiUiMs = nowMs;
            update_status_label();
        }
    }
    {
        static uint32_t s_lastSettingsMs = 0;
        const uint32_t nowMs = millis();
        if ((nowMs - s_lastSettingsMs) >= 1000U) {
            s_lastSettingsMs = nowMs;
            ui_config_refresh_timer_label();
            if (ui_settings_service(&appState)) {
                apply_and_report();
                ui_app_sync_from_state();
            }
        }
    }

    if (!led_calib_is_active()) {
        if (appState.dirty) {
            led_controller_apply(&appState);
        }

        if (lamp_state_music_active(&appState) && appState.power) {
            music_effects_update(&appState);
        } else {
            led_controller_service();
        }
    }

    ir_control_service();

    if (onConfigScreen && audio_input_is_running()) {
        static uint32_t s_lastMicUiMs = 0;
        const uint32_t now = millis();
        if ((now - s_lastMicUiMs) >= 80U) {
            s_lastMicUiMs = now;
            ui_config_update_mic_sensitivity_meter(audio_input_get_sound_level(),
                                                   audio_input_get_music_level());
        }
    }

#if ENABLE_RAINMAKER
#if MIC_START_AT_BOOT && AUDIO_SERIAL_PLOTTER
    if (!audioStarted && !onProvScreen && !rainmaker_app_provisioning_active() &&
        rainmakerStarted && rainmaker_app_init_done()) {
        audioStarted = true;
        audio_input_start();
    }
#endif
    if (rainmakerStarted && rainmaker_app_provisioning_active() && audio_input_is_running()) {
        audio_input_stop();
        audioStarted = false;
    }
#if AUDIO_START_ON_PARTY || AUDIO_START_ON_MUSIC_BAR
    if (!audioStarted && lamp_state_music_active(&appState) && appState.power && !onProvScreen &&
        !rainmaker_app_provisioning_active()) {
        audioStarted = true;
        audio_input_start();
    }
#endif
    if (!audioStarted && rainmakerStarted && rainmaker_app_init_done() && !onProvScreen) {
        if (rainmaker_app_provisioning_active() || rainmaker_app_prov_started()) {
            /* sin audio durante prov */
        } else
#if AUDIO_START_AFTER_WIFI
        if (rainmaker_app_is_online()) {
            audioStarted = true;
            audio_input_start();
        } else if ((millis() - rainmakerStartMs) >= AUDIO_START_FALLBACK_MS) {
            audioStarted = true;
            audio_input_start();
        }
#else
        if ((int32_t)(millis() - audioStartMs) >= 0) {
            audioStarted = true;
            audio_input_start();
        }
#endif
    }
#else
#if MIC_START_AT_BOOT && AUDIO_SERIAL_PLOTTER
    if (!audioStarted && (int32_t)(millis() - audioStartMs) >= 0) {
        audioStarted = true;
        audio_input_start();
    }
#endif
#if AUDIO_START_ON_PARTY || AUDIO_START_ON_MUSIC_BAR
    if (!audioStarted && lamp_state_music_active(&appState) && appState.power) {
        audioStarted = true;
        audio_input_start();
    }
#endif
    if (!audioStarted && (int32_t)(millis() - audioStartMs) >= 0) {
        audioStarted = true;
        audio_input_start();
    }
#endif
    delay(2);
}
