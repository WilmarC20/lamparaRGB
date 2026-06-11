#include "config.h"
#include "ui_app.h"
#include "lv_conf.h"
#include "lamp_log.h"
#include <Arduino.h>
#include <esp_log.h>
#if ENABLE_SPEAKER
#include "audio_output.h"
#endif

#if LV_MEM_CUSTOM != 1
#error "lv_conf.h incorrecto: LV_MEM_CUSTOM debe ser 1. Usa ui/lv_conf.h (sketch ui/)"
#endif

void setup()
{
#if ENABLE_SPEAKER
    audio_output_boot_silence();
#endif
#if LAMP_SERIAL_LOG
    Serial.begin(115200);
    delay(500);
#else
    /* GPIO 1 (TX) es el data de la cinta: silenciar tambien los logs
     * runtime de ESP-IDF (mqtt, wifi) para que no entren a los LEDs. */
    esp_log_level_set("*", ESP_LOG_NONE);
#endif
    LAMP_LOG("LamparaV3 iniciando... heap=%u\n", ESP.getFreeHeap());
#if USE_LVGL_UI
    ui_app_setup();
    LAMP_LOG("Setup OK heap=%u\n", ESP.getFreeHeap());
#endif
}

void loop()
{
#if USE_LVGL_UI
    ui_app_loop();
#else
    delay(1000);
#endif
}
