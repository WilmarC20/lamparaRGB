#include "rainmaker_app.h"
#include "config.h"
#include "lamp_log.h"
#include "led_controller.h"
#include "music_effects.h"
#include "ui_app.h"
#include <Arduino.h>
#include <Preferences.h>
#include <esp_partition.h>
#include <string.h>

#if ENABLE_RAINMAKER

#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <network_provisioning/manager.h>
#include <esp_rmaker_mqtt.h>

#define RM_PREFS_NS          "lamp_v3"
#define RM_PREFS_KEY_PROV_UI "prov_ui"
#define RM_PROV_UI_MAGIC     0xA70C42u

static const char *service_name = "PROV_ESP32";
static const char *pop = "12345678";

static lamp_state_t *gState = NULL;
static Node lampNode;
static Device *lampDevice = NULL;
static volatile bool rmApplyPending = false;
static bool s_initDone = false;
static bool s_provOnly = false;
static bool s_wifiProvisioned = false;
static bool s_cloudPaused = false;
static uint32_t s_resumeReportMs = 0;

static Param s_powerParam("Power", ESP_RMAKER_PARAM_POWER, value(true),
                          PROP_FLAG_READ | PROP_FLAG_WRITE);
static Param s_brightnessParam("Brightness", ESP_RMAKER_PARAM_BRIGHTNESS, value(50),
                               PROP_FLAG_READ | PROP_FLAG_WRITE);
static Param s_hueParam("Hue", ESP_RMAKER_PARAM_HUE, value(180),
                        PROP_FLAG_READ | PROP_FLAG_WRITE);
static Param s_satParam("Saturation", ESP_RMAKER_PARAM_SATURATION, value(100),
                        PROP_FLAG_READ | PROP_FLAG_WRITE);
static Param s_effectParam("Effect", ESP_RMAKER_PARAM_MODE, value("Solido"),
                           PROP_FLAG_READ | PROP_FLAG_WRITE);
static Param s_partyParam("PartyMode", ESP_RMAKER_PARAM_TOGGLE, value(false),
                          PROP_FLAG_READ | PROP_FLAG_WRITE);

static const char *kEffectOptions[] = {
    "Solido", "Arcoiris", "Persecucion", "Onda", "Respiracion", "Estrobo",
    "Musica Barra", "Musica Fiesta", "Musica Persecucion", "Musica Colombia",
    "Musica Respiracion", "Musica Estrobo", "Musica Beat",
};

static const int RM_EFFECT_OPTION_COUNT =
    (int)(sizeof(kEffectOptions) / sizeof(kEffectOptions[0]));

static const char *rm_music_fx_option_name(music_fx_t fx)
{
    switch (fx) {
        case MUSIC_FX_BAR:    return "Musica Barra";
        case MUSIC_FX_PARTY:  return "Musica Fiesta";
        case MUSIC_FX_CHASE:  return "Musica Persecucion";
        case MUSIC_FX_WAVE:   return "Musica Colombia";
        case MUSIC_FX_BREATH: return "Musica Respiracion";
        case MUSIC_FX_STROBE: return "Musica Estrobo";
        case MUSIC_FX_BEAT:   return "Musica Beat";
        default:              return NULL;
    }
}

static music_fx_t rm_music_fx_from_option(const char *name)
{
    if (!name) return MUSIC_FX_NONE;
    for (int fx = (int)MUSIC_FX_BAR; fx < (int)MUSIC_FX_COUNT; fx++) {
        const char *opt = rm_music_fx_option_name((music_fx_t)fx);
        if (opt && strcmp(name, opt) == 0) {
            return (music_fx_t)fx;
        }
    }
    return MUSIC_FX_NONE;
}

static bool rm_effect_name_valid(const char *name)
{
    if (!name) return false;
    for (int i = 0; i < RM_EFFECT_OPTION_COUNT; i++) {
        if (strcmp(name, kEffectOptions[i]) == 0) {
            return true;
        }
    }
    return false;
}

static const char *rm_effect_name_for_state(const lamp_state_t *state)
{
    if (!state) return "Solido";
    if (state->musicFx != MUSIC_FX_NONE) {
        const char *name = rm_music_fx_option_name(state->musicFx);
        if (name) return name;
    }
    return led_effect_name(state->effect);
}

static bool rm_apply_effect_name(lamp_state_t *state, const char *name)
{
    if (!state || !name) return false;

    const music_fx_t musicFx = rm_music_fx_from_option(name);
    if (musicFx != MUSIC_FX_NONE) {
        state->musicFx = musicFx;
        state->musicMode = true;
        music_effects_reset();
        return true;
    }

    for (int i = 0; i < LAMP_EFFECT_COUNT; i++) {
        if (strcmp(name, led_effect_name((lamp_effect_t)i)) == 0) {
            state->musicMode = false;
            state->musicFx = MUSIC_FX_NONE;
            state->effect = (lamp_effect_t)i;
            return true;
        }
    }
    return false;
}

static void write_callback(Device *device, Param *param, const param_val_t val,
                           void *priv_data, write_ctx_t *ctx);

static bool rainmaker_register_lamp_device(void)
{
    if (lampDevice) {
        return true;
    }

    lampDevice = new Device("Luz LED", "esp.device.light", NULL);
    if (!lampDevice) {
        return false;
    }

    s_brightnessParam.addBounds(value(0), value(100), value(1));
    s_brightnessParam.addUIType(ESP_RMAKER_UI_SLIDER);
    s_hueParam.addBounds(value(0), value(360), value(1));
    s_hueParam.addUIType(ESP_RMAKER_UI_HUE_CIRCLE);
    s_satParam.addBounds(value(0), value(100), value(1));
    s_satParam.addUIType(ESP_RMAKER_UI_SLIDER);
    s_effectParam.addValidStrList(kEffectOptions, RM_EFFECT_OPTION_COUNT);
    s_effectParam.addUIType(ESP_RMAKER_UI_DROPDOWN);
    s_partyParam.addUIType(ESP_RMAKER_UI_TOGGLE);
    s_powerParam.addUIType(ESP_RMAKER_UI_TOGGLE);

    lampDevice->addParam(s_powerParam);
    lampDevice->addParam(s_brightnessParam);
    lampDevice->addParam(s_hueParam);
    lampDevice->addParam(s_satParam);
    lampDevice->addParam(s_effectParam);
    lampDevice->addParam(s_partyParam);
    lampDevice->addCb(write_callback);
    lampNode.addDevice(*lampDevice);
    return true;
}

static void write_callback(Device *device, Param *param, const param_val_t val,
                           void *priv_data, write_ctx_t *ctx)
{
    (void)device;
    (void)priv_data;
    (void)ctx;
    if (!gState || !param) return;

    const char *param_name = param->getParamName();

    if (strcmp(param_name, "Power") == 0) {
        gState->power = val.val.b;
    } else if (strcmp(param_name, "Brightness") == 0) {
        gState->brightness = (uint8_t)((val.val.i * 255) / 100);
    } else if (strcmp(param_name, "Hue") == 0) {
        gState->hue = (uint16_t)val.val.i;
    } else if (strcmp(param_name, "Saturation") == 0) {
        gState->saturation = (uint8_t)((val.val.i * 255) / 100);
    } else if (strcmp(param_name, "Effect") == 0) {
        if (!rm_apply_effect_name(gState, val.val.s)) {
            return;
        }
        rmApplyPending = true;
        param->updateAndReport(val);
        s_partyParam.updateAndReport(value(gState->musicMode));
        return;
    } else if (strcmp(param_name, "PartyMode") == 0) {
        ui_app_set_music_mode(val.val.b);
        rmApplyPending = true;
        param->updateAndReport(val);
        const char *eff = rm_effect_name_for_state(gState);
        if (!rm_effect_name_valid(eff)) {
            eff = "Solido";
        }
        s_effectParam.updateAndReport(esp_rmaker_str(eff));
        return;
    } else {
        return;
    }

    rmApplyPending = true;
    param->updateAndReport(val);
}

void sysProvEvent3(arduino_event_t *event)
{
    switch (event->event_id) {
        case ARDUINO_EVENT_PROV_START:
            if (!s_provOnly) {
                break;
            }
            Serial.println("Provisioning iniciado");
#if CONFIG_IDF_TARGET_ESP32
            printQR(service_name, pop, "ble");
#else
            printQR(service_name, pop, "softap");
#endif
            break;

        case ARDUINO_EVENT_PROV_INIT:
            WiFiProv.disableAutoStop(10000);
            break;

        case ARDUINO_EVENT_PROV_CRED_SUCCESS:
            WiFiProv.endProvision();
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            LAMP_LOG_LN("RM: WiFi conectado");
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            LAMP_LOG_LN("RM: WiFi desconectado");
            break;

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            LAMP_LOG_LN("RM: WiFi asociado");
            break;

        default:
            break;
    }
}

static bool rm_prefs_set_prov_ui(void)
{
    Preferences prefs;
    if (!prefs.begin(RM_PREFS_NS, false)) {
        return false;
    }
    prefs.putUInt(RM_PREFS_KEY_PROV_UI, RM_PROV_UI_MAGIC);
    prefs.end();
    return true;
}

static bool rm_prefs_consume_prov_ui(void)
{
    Preferences prefs;
    if (!prefs.begin(RM_PREFS_NS, false)) {
        return false;
    }
    const uint32_t v = prefs.getUInt(RM_PREFS_KEY_PROV_UI, 0);
    if (v == RM_PROV_UI_MAGIC) {
        prefs.remove(RM_PREFS_KEY_PROV_UI);
        prefs.end();
        return true;
    }
    prefs.end();
    return false;
}

static void rm_prefs_clear_prov_ui(void)
{
    Preferences prefs;
    if (!prefs.begin(RM_PREFS_NS, false)) {
        return;
    }
    prefs.remove(RM_PREFS_KEY_PROV_UI);
    prefs.end();
}

bool rainmaker_app_consume_boot_prov_request(void)
{
    return false;
}

bool rainmaker_app_consume_prov_ui_boot(void)
{
    if (!rm_prefs_consume_prov_ui()) {
        return false;
    }
    s_provOnly = true;
    return true;
}

bool rainmaker_app_prov_ui_active(void)
{
    return s_provOnly;
}

bool rainmaker_app_setup_blocks_ui(void)
{
    return s_provOnly;
}

static void rm_wifi_init_provision_manager(void)
{
#if CONFIG_IDF_TARGET_ESP32
    WiFiProv.initProvision(NETWORK_PROV_SCHEME_BLE, NETWORK_PROV_SCHEME_HANDLER_FREE_BTDM);
#else
    WiFiProv.initProvision(NETWORK_PROV_SCHEME_SOFTAP, NETWORK_PROV_SCHEME_HANDLER_NONE);
#endif
}

static bool rm_wifi_provisioned(void)
{
    bool provisioned = false;
    if (network_prov_mgr_is_wifi_provisioned(&provisioned) != ESP_OK) {
        return false;
    }
    return provisioned;
}

static void rm_wifi_begin(void)
{
#if CONFIG_IDF_TARGET_ESP32
    WiFiProv.beginProvision(
        NETWORK_PROV_SCHEME_BLE,
        NETWORK_PROV_SCHEME_HANDLER_FREE_BTDM,
        NETWORK_PROV_SECURITY_1,
        pop,
        service_name);
#else
    WiFiProv.beginProvision(
        NETWORK_PROV_SCHEME_SOFTAP,
        NETWORK_PROV_SCHEME_HANDLER_NONE,
        NETWORK_PROV_SECURITY_1,
        pop,
        service_name);
#endif
}

void rainmaker_app_setup(lamp_state_t *state)
{
    if (s_initDone) {
        return;
    }

    gState = state;

    lampNode = RMaker.initNode("MiESP32");
    rainmaker_register_lamp_device();

    RMaker.start();

    RMaker.setTimeZone("America/Bogota");

    WiFi.onEvent(sysProvEvent3);
    rm_wifi_init_provision_manager();
    s_wifiProvisioned = rm_wifi_provisioned();

    if (s_provOnly) {
        LAMP_LOG_LN("RM: modo prov UI (BLE + QR)");
        rm_wifi_begin();
    } else if (rm_wifi_provisioned()) {
        LAMP_LOG_LN("RM: conectando WiFi guardado");
        rm_wifi_begin();
    } else {
        LAMP_LOG_LN("RM: sin WiFi guardado (prov desde Ajustes)");
    }

    /* No liberar BT manualmente: el handler FREE_BTDM del provisioning ya
     * devuelve esa RAM, y hacerlo dos veces corrompe el heap (canario
     * 0xbaad5678 en crash LoadProhibited). Verificado: solo retenia 2.5 KB. */

    s_initDone = true;
}

void rainmaker_app_loop(void)
{
    if (s_provOnly) {
        return;
    }

    if (rmApplyPending && gState) {
        rmApplyPending = false;
        lamp_state_mark_dirty(gState);
        ui_app_notify_state_changed();
    }

    /* Tras reanudar la nube, re-sincronizar params cuando MQTT ya reconecto */
    if (s_resumeReportMs != 0 && (int32_t)(millis() - s_resumeReportMs) >= 0) {
        s_resumeReportMs = 0;
        if (gState) {
            rainmaker_app_report_state(gState);
        }
    }
}

void rainmaker_app_pause_cloud(void)
{
    if (!s_initDone || s_provOnly || s_cloudPaused) {
        return;
    }
    s_cloudPaused = true;
    s_resumeReportMs = 0;
    /* esp_rmaker_stop solo detiene el agente: la conexion MQTT/TLS (~40 KB)
     * queda viva. Desconectar explicito para que el transporte libere RAM. */
    const esp_err_t stopErr = RMaker.stop();
    const esp_err_t mqttErr = esp_rmaker_mqtt_disconnect();
    LAMP_LOG("RM: nube pausada stop=%d mqtt=%d heap=%u\n",
             (int)stopErr, (int)mqttErr, ESP.getFreeHeap());
}

void rainmaker_app_resume_cloud(void)
{
    if (!s_cloudPaused) {
        return;
    }
    s_cloudPaused = false;
    const esp_err_t startErr = RMaker.start();
    const esp_err_t mqttErr = esp_rmaker_mqtt_connect();
    /* MQTT reconecta async; reportar estado unos segundos despues */
    s_resumeReportMs = millis() + 6000U;
    LAMP_LOG("RM: nube reanudada start=%d mqtt=%d heap=%u\n",
             (int)startErr, (int)mqttErr, ESP.getFreeHeap());
}

bool rainmaker_app_cloud_paused(void)
{
    return s_cloudPaused;
}

void rainmaker_app_report_state(const lamp_state_t *state)
{
    if (!state || s_cloudPaused || WiFi.status() != WL_CONNECTED) {
        return;
    }

    const int brillo_rm = (int)((state->brightness * 100) / 255);
    const int sat_rm = (int)((state->saturation * 100) / 255);

    s_powerParam.updateAndReport(value(state->power));
    s_brightnessParam.updateAndReport(value(brillo_rm));
    s_hueParam.updateAndReport(value((int)state->hue));
    s_satParam.updateAndReport(value(sat_rm));

    const char *eff = rm_effect_name_for_state(state);
    if (!rm_effect_name_valid(eff)) {
        eff = "Solido";
    }
    s_effectParam.updateAndReport(esp_rmaker_str(eff));
    s_partyParam.updateAndReport(value(state->musicMode));
}

bool rainmaker_app_is_online(void)
{
    return WiFi.status() == WL_CONNECTED;
}

bool rainmaker_app_provisioning_active(void)
{
    return s_provOnly && !rainmaker_app_is_online();
}

bool rainmaker_app_init_done(void)
{
    return s_initDone;
}

bool rainmaker_app_prov_started(void)
{
    return s_initDone;
}

bool rainmaker_app_get_qr_payload(char *buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return false;
    }
#if CONFIG_IDF_TARGET_ESP32
    const char *transport = "ble";
#else
    const char *transport = "softap";
#endif
    snprintf(buf, buf_len,
             "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"%s\"}",
             service_name, pop, transport);
    return true;
}

void rainmaker_app_get_service_name(char *buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return;
    }
    snprintf(buf, buf_len, "%s", service_name);
}

void rainmaker_app_print_qr_serial(void)
{
#if CONFIG_IDF_TARGET_ESP32
    printQR(service_name, pop, "ble");
#else
    printQR(service_name, pop, "softap");
#endif
}

void rainmaker_app_print_qr_serial_async(void) {}

bool rainmaker_app_wifi_provisioned(void)
{
    return s_wifiProvisioned;
}

bool rainmaker_app_wifi_credentials_saved(void)
{
    return rainmaker_app_is_online();
}

void rainmaker_app_on_ui_ready(void)
{
    if (gState && rainmaker_app_is_online()) {
        rainmaker_app_report_state(gState);
    }
}

void rainmaker_app_wifi_reset(void)
{
    rm_prefs_clear_prov_ui();
    RMakerWiFiReset(2);
}

void rainmaker_app_request_prov_ui_restart(void)
{
    rm_prefs_set_prov_ui();
    Serial.flush();
    delay(200);
    ESP.restart();
}

void rainmaker_app_exit_prov_ui(void)
{
    rm_prefs_clear_prov_ui();
    Serial.flush();
    delay(200);
    ESP.restart();
}

void rainmaker_app_begin_provision(void)
{
    rainmaker_app_request_prov_ui_restart();
}

void rainmaker_app_cancel_provision_request(void)
{
    rm_prefs_clear_prov_ui();
}

bool rainmaker_app_start_ble_before_ui(void)
{
    return true;
}

void rainmaker_app_run_boot_provision(void) {}

bool rainmaker_app_start_live_provision(void)
{
    return s_initDone;
}

void rainmaker_app_ble_start_async(void) {}

bool rainmaker_app_ble_start_poll(bool *done, bool *ok)
{
    if (done) {
        *done = true;
    }
    if (ok) {
        *ok = s_initDone;
    }
    return true;
}

bool rainmaker_app_consume_prov_status(char *buf, size_t buf_len, uint32_t *color_hex)
{
    static uint8_t s_lastPhase = 0;

    if (!buf || buf_len == 0) {
        return false;
    }

    const uint8_t phase = rainmaker_app_is_online() ? 2U : 1U;
    if (phase == s_lastPhase) {
        return false;
    }
    s_lastPhase = phase;

    if (phase == 2U) {
        strncpy(buf, "Conectado - pulsa Salir", buf_len - 1);
        if (color_hex) {
            *color_hex = 0x00FFAA;
        }
    } else {
        strncpy(buf, "Escanea QR con la app", buf_len - 1);
        if (color_hex) {
            *color_hex = 0xFFAA44;
        }
    }
    buf[buf_len - 1] = '\0';
    return true;
}

#else

void rainmaker_app_setup(lamp_state_t *state) { (void)state; }
bool rainmaker_app_start_ble_before_ui(void) { return false; }
bool rainmaker_app_consume_boot_prov_request(void) { return false; }
bool rainmaker_app_consume_prov_ui_boot(void) { return false; }
bool rainmaker_app_prov_ui_active(void) { return false; }
void rainmaker_app_request_prov_ui_restart(void) {}
bool rainmaker_app_start_live_provision(void) { return false; }
void rainmaker_app_ble_start_async(void) {}
bool rainmaker_app_ble_start_poll(bool *done, bool *ok)
{
    if (done) *done = true;
    if (ok) *ok = false;
    return true;
}
void rainmaker_app_exit_prov_ui(void) {}
bool rainmaker_app_consume_prov_status(char *buf, size_t buf_len, uint32_t *color_hex)
{
    (void)buf; (void)buf_len; (void)color_hex;
    return false;
}
void rainmaker_app_run_boot_provision(void) {}
bool rainmaker_app_setup_blocks_ui(void) { return false; }
void rainmaker_app_begin_provision(void) {}
void rainmaker_app_cancel_provision_request(void) {}
bool rainmaker_app_init_done(void) { return true; }
bool rainmaker_app_prov_started(void) { return false; }
void rainmaker_app_print_qr_serial(void) {}
void rainmaker_app_print_qr_serial_async(void) {}
void rainmaker_app_loop(void) {}
void rainmaker_app_report_state(const lamp_state_t *state) { (void)state; }
void rainmaker_app_pause_cloud(void) {}
void rainmaker_app_resume_cloud(void) {}
bool rainmaker_app_cloud_paused(void) { return false; }
bool rainmaker_app_is_online(void) { return false; }
bool rainmaker_app_provisioning_active(void) { return false; }
bool rainmaker_app_get_qr_payload(char *buf, size_t buf_len)
{
    (void)buf; (void)buf_len;
    return false;
}
void rainmaker_app_get_service_name(char *buf, size_t buf_len)
{
    if (buf && buf_len) buf[0] = '\0';
}
bool rainmaker_app_wifi_credentials_saved(void) { return false; }
bool rainmaker_app_wifi_provisioned(void) { return false; }
void rainmaker_app_on_ui_ready(void) {}
void rainmaker_app_wifi_reset(void) {}

#endif
