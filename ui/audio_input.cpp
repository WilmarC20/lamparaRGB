#include "audio_input.h"
#include "config.h"
#include "pins.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>

#ifndef AUDIO_SAMPLE_MS
#define AUDIO_SAMPLE_MS 1
#endif
#ifndef AUDIO_BLOCK_LEN
#define AUDIO_BLOCK_LEN 32
#endif
#ifndef AUDIO_EMA_SHIFT
#define AUDIO_EMA_SHIFT 3
#endif

#define ADC_TASK_STACK     2560
#define ADC_TASK_PRIO      1

static int16_t sBuffer[AUDIO_BLOCK_LEN];
static audio_levels_t latestLevels;
static portMUX_TYPE levelsMux = portMUX_INITIALIZER_UNLOCKED;
static TaskHandle_t adcTaskHandle = NULL;

static volatile int s_soundLevel = 0;
static volatile int s_soundSpan = 0;
static volatile int s_rawAdc = 0;
static volatile int s_dcOffset = 2048;
static bool s_testMode = false;
static int s_micThreshold = AUDIO_MIC_THRESHOLD;
static int s_silenceLevel = AUDIO_SILENCE_LEVEL;

static int level_from_abs(int acAbs)
{
    int level = acAbs - s_micThreshold;
    if (level < 0) level = 0;
    return (level * 3) / 2;
}

static void compute_levels(const int16_t *samples, size_t count, audio_levels_t *out)
{
    double sumSq = 0;
    int32_t sMin = 0;
    int32_t sMax = 0;

    for (size_t i = 0; i < count; i++) {
        const int16_t s = samples[i];
        sumSq += (double)s * (double)s;
        if (i == 0) {
            sMin = sMax = s;
        } else {
            if (s < sMin) sMin = s;
            if (s > sMax) sMax = s;
        }
    }

    out->rms = (float)sqrt(sumSq / (double)count);
    s_soundSpan = (int)(sMax - sMin);

    for (int b = 0; b < AUDIO_BAND_COUNT; b++) {
        out->bands[b] = 0;
    }

    size_t bandSize = count / AUDIO_BAND_COUNT;
    if (bandSize == 0) bandSize = 1;
    for (int b = 0; b < AUDIO_BAND_COUNT; b++) {
        int bandSum = 0;
        size_t start = (size_t)b * bandSize;
        size_t end = (b == AUDIO_BAND_COUNT - 1) ? count : start + bandSize;
        for (size_t i = start; i < end; i++) {
            bandSum += abs((int)samples[i]);
        }
        out->bands[b] = (float)bandSum / (float)(end - start);
    }
}

static void adc_task(void *param)
{
    (void)param;
    TickType_t period = pdMS_TO_TICKS(AUDIO_SAMPLE_MS);
    if (period < 1) {
        period = 1;
    }

    size_t idx = 0;
    TickType_t lastWake = xTaskGetTickCount();
    int dc = 2048;
    int emaAbs = 0;

    while (true) {
        const int raw = analogRead(PIN_MIC_ADC);
        s_rawAdc = raw;

        dc = (dc * 31 + raw) / 32;
        s_dcOffset = dc;

        const int ac = raw - dc;
        const int acAbs = abs(ac);

        emaAbs += (acAbs - emaAbs) >> AUDIO_EMA_SHIFT;
        s_soundLevel = level_from_abs(emaAbs);

        sBuffer[idx++] = (int16_t)ac;

        if (idx >= AUDIO_BLOCK_LEN) {
            audio_levels_t levels;
            compute_levels(sBuffer, AUDIO_BLOCK_LEN, &levels);

            portENTER_CRITICAL(&levelsMux);
            latestLevels = levels;
            portEXIT_CRITICAL(&levelsMux);
            idx = 0;
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

void audio_input_init(void)
{
    memset(&latestLevels, 0, sizeof(latestLevels));
}

void audio_input_set_test_mode(bool enable)
{
    s_testMode = enable;
}

bool audio_input_is_test_mode(void)
{
    return s_testMode;
}

void audio_input_start(void)
{
    if (adcTaskHandle != NULL) return;

    memset(&latestLevels, 0, sizeof(latestLevels));
    s_soundLevel = 0;
    s_soundSpan = 0;
    s_dcOffset = 2048;
    analogSetPinAttenuation(PIN_MIC_ADC, ADC_11db);
    analogReadResolution(12);

    xTaskCreatePinnedToCore(adc_task, "adc_audio", ADC_TASK_STACK, NULL, ADC_TASK_PRIO,
                            &adcTaskHandle, 1);
}

void audio_input_stop(void)
{
    s_testMode = false;
    if (adcTaskHandle) {
        vTaskDelete(adcTaskHandle);
        adcTaskHandle = NULL;
    }
    portENTER_CRITICAL(&levelsMux);
    memset(&latestLevels, 0, sizeof(latestLevels));
    portEXIT_CRITICAL(&levelsMux);
    s_soundLevel = 0;
    s_soundSpan = 0;
}

bool audio_input_is_running(void)
{
    return adcTaskHandle != NULL;
}

bool audio_input_get_levels(audio_levels_t *out)
{
    if (!out) return false;
    portENTER_CRITICAL(&levelsMux);
    *out = latestLevels;
    portEXIT_CRITICAL(&levelsMux);
    return true;
}

int audio_input_get_sound_level(void)
{
    return s_soundLevel;
}

int audio_input_get_music_level(void)
{
    int level = s_soundLevel - s_silenceLevel;
    if (level < 0) {
        level = 0;
    }
    return level;
}

int audio_input_get_sound_span(void)
{
    return s_soundSpan;
}

int audio_input_get_raw_adc(void)
{
    return s_rawAdc;
}

void audio_input_set_silence_level(int level)
{
    if (level < 0) {
        level = 0;
    }
    if (level > 400) {
        level = 400;
    }
    s_silenceLevel = level;
}

int audio_input_get_silence_level(void)
{
    return s_silenceLevel;
}

int audio_input_silence_from_sensitivity_pct(uint8_t pct)
{
    return 350 - ((int)pct * 300 / 100);
}

uint8_t audio_input_sensitivity_pct_from_silence(int silence)
{
    if (silence <= 50) {
        return 100;
    }
    if (silence >= 350) {
        return 0;
    }
    return (uint8_t)((350 - silence) * 100 / 300);
}
