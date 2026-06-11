#include "audio_output.h"
#include "config.h"
#include "pins.h"
#include "audio_input.h"
#include "led_controller.h"
#include "lamp_log.h"
#if ENABLE_RADIO
#include "radio_player.h"
#endif
#include <Arduino.h>
#include <esp32-hal-dac.h>

#if ENABLE_SPEAKER

static TaskHandle_t s_playTask = NULL;

static const uint8_t kSin64[64] = {
    128, 140, 152, 164, 176, 188, 199, 210, 219, 228, 235, 242, 247, 251, 254, 255,
    255, 254, 251, 247, 242, 235, 228, 219, 210, 199, 188, 176, 164, 152, 140, 128,
    115, 103,  91,  79,  67,  56,  45,  34,  25,  16,   9,   2,   1,   1,   1,   0,
      0,   1,   1,   1,   2,   9,  16,  25,  34,  45,  56,  67,  79,  91, 103, 115,
};

static void speaker_silence(void)
{
    dacWrite(PIN_SPEAKER, 128);
}

/* Reposo: DAC apagado y pin a GND. Con el DAC en media escala el FM8002A
 * queda polarizado amplificando el ruido de rail (LEDs, WiFi, tactil). */
static void speaker_quiet(void)
{
    dacDisable(PIN_SPEAKER);
    pinMode(PIN_SPEAKER, OUTPUT);
    digitalWrite(PIN_SPEAKER, LOW);
}

static void play_tone_dac(uint16_t freq_hz, uint16_t duration_ms, uint8_t amplitude)
{
    if (freq_hz == 0 || duration_ms == 0) {
        return;
    }

    const int sample_rate = 8000;
    const int period_samples = sample_rate / (int)freq_hz;
    if (period_samples < 2) {
        return;
    }

    const int us_per_sample = 1000000 / sample_rate;
    const unsigned long end_ms = millis() + duration_ms;
    int sample_idx = 0;

    while ((long)(millis() - end_ms) < 0) {
        const int table_idx = (sample_idx * 64) / period_samples % 64;
        const int centered = (int)kSin64[table_idx] - 128;
        int out = 128 + (centered * (int)amplitude) / 128;
        if (out < 0) {
            out = 0;
        }
        if (out > 255) {
            out = 255;
        }
        dacWrite(PIN_SPEAKER, (uint8_t)out);
        delayMicroseconds(us_per_sample);
        sample_idx++;
    }
}

static void speaker_test_task(void *param)
{
    (void)param;

    const bool micWasRunning = audio_input_is_running();
    if (micWasRunning) {
        audio_input_stop();
    }
#if ENABLE_LED_STRIP
    led_controller_set_output_paused(true);
#endif

    static const struct {
        uint16_t freq_hz;
        uint16_t duration_ms;
    } kNotes[] = {
        {523, 180},
        {659, 180},
        {784, 180},
        {1047, 420},
    };

    LAMP_LOG_LN("SPK: prueba altavoz");
    for (unsigned i = 0; i < (sizeof(kNotes) / sizeof(kNotes[0])); i++) {
        play_tone_dac(kNotes[i].freq_hz, kNotes[i].duration_ms, 72);
        speaker_silence();
        vTaskDelay(pdMS_TO_TICKS(25));
    }

    speaker_quiet();
#if ENABLE_LED_STRIP
    led_controller_set_output_paused(false);
#endif
    if (micWasRunning) {
        audio_input_start();
    }

    s_playTask = NULL;
    vTaskDelete(NULL);
}

void audio_output_boot_silence(void)
{
    pinMode(PIN_SPEAKER, OUTPUT);
    digitalWrite(PIN_SPEAKER, LOW);
#if !ENABLE_RADIO
    dacDisable(PIN_SPEAKER);
#endif
}

void audio_output_init(void)
{
    speaker_quiet();
}

bool audio_output_is_playing(void)
{
    return s_playTask != NULL;
}

void audio_output_play_test(void)
{
    if (s_playTask != NULL) {
        return;
    }
#if ENABLE_RADIO
    radio_player_stop();
#endif
    xTaskCreatePinnedToCore(speaker_test_task, "spk_test", 3072, NULL, 1, &s_playTask, 0);
}

#else

void audio_output_boot_silence(void) {}
void audio_output_init(void) {}
bool audio_output_is_playing(void) { return false; }
void audio_output_play_test(void) {}

#endif
