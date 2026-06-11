#include "radio_player.h"
#include "config.h"
#include "pins.h"
#include "audio_input.h"
#include "led_controller.h"
#include "lamp_log.h"
#include "rainmaker_app.h"
#include "audio_output.h"
#include <Arduino.h>
#include <WiFi.h>
#include <new>

#if ENABLE_RADIO && ENABLE_SPEAKER

#ifdef DOUT
#undef DOUT
#endif
#ifdef DIN
#undef DIN
#endif
#include "Audio.h"

/* Inicializacion diferida: evita I2S/DAC legacy antes de main() */
static Audio *s_audio = NULL;
static bool s_playing = false;
static bool s_connecting = false;
static int s_station = -1;
static bool s_micWasRunning = false;
static char s_status[48] = "Detenido";
static uint8_t s_volume_idx = RADIO_VOLUME_PCT_TO_IDX(RADIO_DEFAULT_VOLUME_PCT);

static const struct {
    const char *name;
    const char *url;
} kStations[] = {
    { "W Radio", "http://26653.live.streamtheworld.com/WRADIO_SC" },
    { "Caracol Radio", "http://27323.live.streamtheworld.com/CARACOL_RADIO_SC" },
    { "RCN Radio", "http://playerservices.streamtheworld.com/api/livestream-redirect/RCNRADIO.mp3" },
    { "Blu Radio", "http://27423.live.streamtheworld.com/BLURADIO_SC" },
    { "La FM", "http://playerservices.streamtheworld.com/api/livestream-redirect/LAFM.mp3" },
    { "Tropicana", "http://26563.live.streamtheworld.com/TROPICANA_SC" },
    { "Radioacktiva", "http://24493.live.streamtheworld.com/RADIO_ACTIVA_SC" },
    { "La Kalle", "http://26513.live.streamtheworld.com/LA_KALLE_SC" },
    { "Olimpica Stereo", "http://playerservices.streamtheworld.com/api/livestream-redirect/OLIMPICA_STEREO.mp3" },
    { "Los 40", "http://25353.live.streamtheworld.com/LOS40_SC" },
};

void audio_info(const char *info)
{
    if (!info) {
        return;
    }
    LAMP_LOG("Audio: %s\n", info);
}

/* Tap por muestra de ESP32-audioI2S (pre-volumen): efectos LED musicales
 * reaccionan al audio de la radio en vez del mic. */
void audio_tap_sample(int16_t left, int16_t right)
{
    audio_input_feed_external(left, right);
}

static bool radio_wifi_ready(void)
{
#if ENABLE_RAINMAKER
    return rainmaker_app_is_online();
#else
    return WiFi.status() == WL_CONNECTED;
#endif
}

static void radio_set_status(const char *text)
{
    if (!text) {
        return;
    }
    strncpy(s_status, text, sizeof(s_status) - 1U);
    s_status[sizeof(s_status) - 1U] = '\0';
}

static bool radio_ensure_audio(void)
{
    if (s_audio != NULL) {
        return true;
    }

    audio_input_stop();

    s_audio = new (std::nothrow) Audio(true, 2);
    if (s_audio == NULL) {
        LAMP_LOG_LN("RADIO: sin RAM para Audio");
        return false;
    }

    LAMP_LOG("RADIO: Audio DAC GPIO26 heap=%u\n", ESP.getFreeHeap());
    return true;
}

static void radio_apply_volume(void)
{
    if (s_audio == NULL) {
        return;
    }
    if (s_volume_idx > 21U) {
        s_volume_idx = 21U;
    }
    s_audio->setVolume(s_volume_idx);
    s_audio->setTone(0, 0, 0);
}

static uint8_t radio_pct_to_vol(uint8_t pct)
{
    if (pct > 100U) {
        pct = 100U;
    }
    return (uint8_t)((pct * 21U + 50U) / 100U);
}

static uint8_t radio_vol_to_pct(uint8_t vol)
{
    if (vol > 21U) {
        vol = 21U;
    }
    return (uint8_t)((vol * 100U + 10U) / 21U);
}

static void radio_restore_mic(void)
{
    if (s_micWasRunning) {
        audio_input_start();
    }
    s_micWasRunning = false;
}

static void radio_prepare_playback(void)
{
    if (!s_playing && !s_connecting) {
        s_micWasRunning = audio_input_is_running();
    }
    if (audio_input_is_running()) {
        audio_input_stop();
    }
#if ENABLE_RAINMAKER
    /* Cloud XOR radio: MQTT/TLS y streaming MP3 no caben juntos en RAM. */
    rainmaker_app_pause_cloud();
#endif
    WiFi.setSleep(WIFI_PS_NONE);
    WiFi.setAutoReconnect(true);
}

#if ENABLE_RAINMAKER
/* El teardown de MQTT/TLS tras esp_rmaker_stop es ASINCRONO: esperar a que
 * la RAM liberada (~50 KB) este disponible antes de crear Audio (~50 KB con
 * DMA I2S) + decoder MP3 y socket (~30 KB). Umbral < eso = fallo seguro. */
#define RADIO_HEAP_TARGET   80000U
#define RADIO_HEAP_MINIMUM  60000U
#define RADIO_HEAP_WAIT_MS  4000U

static bool radio_wait_for_heap(void)
{
    for (uint32_t waited = 0; waited < RADIO_HEAP_WAIT_MS; waited += 50U) {
        if (ESP.getFreeHeap() >= RADIO_HEAP_TARGET) {
            break;
        }
        delay(50);
    }
    LAMP_LOG("RADIO: heap tras pausa nube=%u\n", ESP.getFreeHeap());
    return ESP.getFreeHeap() >= RADIO_HEAP_MINIMUM;
}
#endif

void radio_player_init(void)
{
    /* Audio se crea al dar Play, con la nube ya pausada (cloud XOR radio):
     * en reposo quedan ~25 KB mas de heap para RainMaker, LVGL y LEDs. */
    WiFi.setAutoReconnect(true);
}

void radio_player_loop(void)
{
    if (s_audio != NULL) {
        s_audio->loop();
    }
}

void radio_player_service(void)
{
}

bool radio_player_is_playing(void)
{
    return s_playing && s_audio != NULL && s_audio->isRunning();
}

bool radio_player_is_busy(void)
{
    return s_connecting;
}

int radio_player_get_station(void)
{
    return s_station;
}

const char *radio_player_status_text(void)
{
    return s_status;
}

uint8_t radio_player_station_count(void)
{
    return (uint8_t)(sizeof(kStations) / sizeof(kStations[0]));
}

const char *radio_player_station_name(uint8_t idx)
{
    if (idx >= radio_player_station_count()) {
        return "";
    }
    return kStations[idx].name;
}

void radio_player_stop(void)
{
    if (s_audio != NULL) {
        s_audio->stopSong();
        /* Destruir Audio devuelve objeto + InBuff + DMA I2S (~25 KB)
         * para que el TLS/MQTT de RainMaker pueda rehacer su handshake. */
        delete s_audio;
        s_audio = NULL;
#if ENABLE_SPEAKER
        audio_output_boot_silence();
#endif
        LAMP_LOG("RADIO: Audio liberado heap=%u\n", ESP.getFreeHeap());
    }
    s_playing = false;
    s_connecting = false;
    s_station = -1;
    audio_input_set_external_source(false);
    radio_set_status("Detenido");
    WiFi.setSleep(WIFI_PS_MIN_MODEM);
    radio_restore_mic();
#if ENABLE_RAINMAKER
    rainmaker_app_resume_cloud();
#endif
}

bool radio_player_play(uint8_t station_idx)
{
    if (station_idx >= radio_player_station_count()) {
        return false;
    }

    if (!radio_wifi_ready()) {
        radio_set_status("Sin WiFi");
        return false;
    }

    s_connecting = true;
    radio_set_status("Conectando...");
    radio_prepare_playback();

#if ENABLE_RAINMAKER
    /* Solo al crear Audio: si ya existe (cambio de emisora) su RAM esta paga */
    if (s_audio == NULL && !radio_wait_for_heap()) {
        s_connecting = false;
        radio_player_stop();
        radio_set_status("Sin RAM");
        LAMP_LOG_LN("RADIO: nube no libero RAM a tiempo");
        return false;
    }
#endif

    if (!radio_ensure_audio()) {
        s_connecting = false;
        radio_player_stop();
        radio_set_status("Sin RAM");
        return false;
    }
    radio_apply_volume();

    if (!s_audio->connecttohost(kStations[station_idx].url)) {
        s_connecting = false;
        radio_player_stop();
        radio_set_status("Fallo conexion");
        LAMP_LOG("RADIO: fallo %s\n", kStations[station_idx].name);
        return false;
    }

    s_station = (int)station_idx;
    s_playing = true;
    s_connecting = false;
    audio_input_set_external_source(true);
    radio_set_status("En vivo");
    LAMP_LOG("RADIO: %s -> %s\n", kStations[station_idx].name, kStations[station_idx].url);
    return true;
}

void radio_player_toggle(uint8_t station_idx)
{
    if (s_playing && s_station == (int)station_idx) {
        radio_player_stop();
        return;
    }
    radio_player_play(station_idx);
}

void radio_player_set_volume(uint8_t pct)
{
    s_volume_idx = radio_pct_to_vol(pct);
    if (s_audio != NULL && (s_playing || s_audio->isRunning())) {
        radio_apply_volume();
    }
}

uint8_t radio_player_get_volume(void)
{
    return radio_vol_to_pct(s_volume_idx);
}

#else

void radio_player_init(void) {}
void radio_player_loop(void) {}
void radio_player_service(void) {}
bool radio_player_is_playing(void) { return false; }
bool radio_player_is_busy(void) { return false; }
int radio_player_get_station(void) { return -1; }
bool radio_player_play(uint8_t station_idx) { (void)station_idx; return false; }
void radio_player_stop(void) {}
void radio_player_toggle(uint8_t station_idx) { (void)station_idx; }
const char *radio_player_station_name(uint8_t idx) { (void)idx; return ""; }
uint8_t radio_player_station_count(void) { return 0; }
const char *radio_player_status_text(void) { return "Desactivado"; }
void radio_player_set_volume(uint8_t pct) { (void)pct; }
uint8_t radio_player_get_volume(void) { return 0; }

#endif
