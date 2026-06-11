#include "audio_input.h"
#include "config.h"
#include "pins.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>

#if defined(ARDUINO_ARCH_ESP32) && ENABLE_RADIO && ENABLE_SPEAKER
#include "driver/adc.h"
#define AUDIO_USE_LEGACY_ADC 1
#endif

#ifndef AUDIO_USE_LEGACY_ADC
#define AUDIO_USE_LEGACY_ADC 0
#endif

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
static volatile TaskHandle_t adcTaskHandle = NULL;
/* Parada cooperativa: vTaskDelete externo puede matar la tarea dentro de
 * portENTER_CRITICAL y dejar el spinlock tomado (deadlock + WDT reset). */
static volatile bool s_adcStopReq = false;

static volatile int s_soundLevel = 0;
static volatile int s_soundSpan = 0;
static volatile int s_rawAdc = 0;
static volatile int s_dcOffset = 2048;
static bool s_testMode = false;
static int s_micThreshold = AUDIO_MIC_THRESHOLD;
static int s_silenceLevel = AUDIO_SILENCE_LEVEL;

/* Pipeline de analisis comun (definido mas abajo); adc_task lo usa antes */
static void audio_process_ac(int ac);

#if AUDIO_USE_LEGACY_ADC
/* GPIO 35 = ADC1_CH7; mismo driver legacy que I2S/DAC interno de ESP32-audioI2S */
static adc1_channel_t mic_adc_channel(void)
{
    return ADC1_CHANNEL_7;
}

static int mic_adc_read_raw(void)
{
    return (int)adc1_get_raw(mic_adc_channel());
}
#endif

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

    while (!s_adcStopReq) {
#if AUDIO_USE_LEGACY_ADC
        const int raw = mic_adc_read_raw();
#else
        const int raw = analogRead(PIN_MIC_ADC);
#endif
        s_rawAdc = raw;

        dc = (dc * 31 + raw) / 32;
        s_dcOffset = dc;

        const int ac = raw - dc;
        const int acAbs = abs(ac);

        audio_process_ac(ac);

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

    adcTaskHandle = NULL;
    vTaskDelete(NULL);
}

/* --- Analisis comun (mic ~1 kHz o radio ~1.4 kHz decimada) ---
 * Nivel global, graves (LPF 1 polo), brillos (residuo HPF) y beats
 * (energia de graves sobre su promedio lento). AGC por caracteristica. */

#define PROC_PEAK_FLOOR 48
#define PROC_NORM_TOP   304   /* sobre rango util MUSIC_LEVEL_MAX=400 */

static volatile int s_procLevel = 0;
static volatile int s_procBass = 0;
static volatile int s_procHigh = 0;
static volatile uint32_t s_procBeatCount = 0;

static int s_procLpf = 0;
static int s_procLevelEma = 0;
static int s_procBassEma = 0;
static int s_procHighEma = 0;
static int s_procBassAvg = 0;
static uint32_t s_lastBeatMs = 0;

/* AGC por caracteristica con pico Y piso: el AGC de hardware del MAX9814
 * comprime la señal (ema ~= pico siempre), asi que normalizar solo contra
 * el pico da valores planos. Expandir entre valle y pico recupera la
 * dinamica real de la musica. Piso: baja rapido, sube ~20/s. */
typedef struct {
    int peak;
    int fl;
    uint16_t flCnt;
} proc_agc_t;

static proc_agc_t s_agcLevel;
static proc_agc_t s_agcBass;
static proc_agc_t s_agcHigh;

static int proc_normalize(proc_agc_t *a, int ema)
{
    if (ema > a->peak) {
        a->peak = ema;
    } else if (a->peak > PROC_PEAK_FLOOR) {
        a->peak -= ((a->peak - PROC_PEAK_FLOOR) >> 11) + 1;
    }

    if (ema < a->fl) {
        a->fl += (ema - a->fl) >> 3;
        if (a->fl < 0) {
            a->fl = 0;
        }
    } else if (++a->flCnt >= 50U) {
        a->flCnt = 0;
        if (a->fl < ema) {
            a->fl++;
        }
    }
    if (a->fl > a->peak - 16) {
        a->fl = a->peak - 16;
    }

    int range = a->peak - a->fl;
    if (range < 24) {
        range = 24;
    }
    int norm = ((ema - a->fl) * PROC_NORM_TOP) / range;
    if (norm < 0) {
        norm = 0;
    }
    if (norm > 400) {
        norm = 400;
    }
    return norm;
}

static void proc_agc_reset(proc_agc_t *a)
{
    a->peak = PROC_PEAK_FLOOR;
    a->fl = 0;
    a->flCnt = 0;
}

static void proc_reset(void)
{
    s_procLpf = 0;
    s_procLevelEma = 0;
    s_procBassEma = 0;
    s_procHighEma = 0;
    s_procBassAvg = 0;
    proc_agc_reset(&s_agcLevel);
    proc_agc_reset(&s_agcBass);
    proc_agc_reset(&s_agcHigh);
    s_procLevel = 0;
    s_procBass = 0;
    s_procHigh = 0;
}

static void audio_process_ac(int ac)
{
    const int acAbs = ac < 0 ? -ac : ac;

    s_procLevelEma += (acAbs - s_procLevelEma) >> AUDIO_EMA_SHIFT;

    /* Graves: LPF alpha 1/4 (~40-60 Hz con muestreo ~1 kHz) */
    s_procLpf += (ac - s_procLpf) >> 2;
    const int bassAbs = s_procLpf < 0 ? -s_procLpf : s_procLpf;
    s_procBassEma += (bassAbs - s_procBassEma) >> 3;

    /* Brillos: residuo paso-alto */
    const int hp = ac - s_procLpf;
    const int highAbs = hp < 0 ? -hp : hp;
    s_procHighEma += (highAbs - s_procHighEma) >> 3;

    s_procLevel = proc_normalize(&s_agcLevel, s_procLevelEma);
    s_procBass = proc_normalize(&s_agcBass, s_procBassEma);
    s_procHigh = proc_normalize(&s_agcHigh, s_procHighEma);

    /* Beat: energia de graves 1.5x sobre su promedio lento (~0.5 s),
     * con refractario de 200 ms (max ~300 BPM) */
    s_procBassAvg += (s_procBassEma - s_procBassAvg) >> 9;
    const uint32_t now = millis();
    if (s_procBassEma > s_procBassAvg + (s_procBassAvg >> 1) &&
        s_procBassEma > 8 && (now - s_lastBeatMs) > 200U) {
        s_lastBeatMs = now;
        s_procBeatCount++;
    }
}

int audio_input_get_norm_level(void)
{
    return s_procLevel;
}

int audio_input_get_norm_bass(void)
{
    return s_procBass;
}

int audio_input_get_norm_high(void)
{
    return s_procHigh;
}

uint32_t audio_input_get_beat_count(void)
{
    return s_procBeatCount;
}

/* --- Fuente externa: PCM de la radio (pre-volumen) en vez del mic --- */

/* 1 de cada 32 muestras (~1.4 kHz a 44.1k, similar a la cadencia del mic) */
#define EXT_DECIMATE 32
/* 16-bit con signo -> rango ~+-512, comparable al AC del ADC de 12 bits */
#define EXT_SHIFT    6

/* AGC: piso del pico (evita division ~0 y dispara con señal minima) */
#define EXT_PEAK_FLOOR  64
/* Nivel cuando la señal toca su pico (max util MUSIC_LEVEL_MAX=400).
 * 304 = 380 * 0.8: sensibilidad reducida 20% a pedido del usuario. */
#define EXT_LEVEL_TOP   304

static volatile bool s_extActive = false;
static int16_t s_extBuf[AUDIO_BLOCK_LEN];
static size_t s_extIdx = 0;
static uint32_t s_extDecim = 0;
static int s_extEma = 0;
static int s_extPeak = EXT_PEAK_FLOOR;

void audio_input_set_external_source(bool enable)
{
    s_extActive = enable;
    s_extIdx = 0;
    s_extDecim = 0;
    s_extEma = 0;
    s_extPeak = EXT_PEAK_FLOOR;
    proc_reset();
    portENTER_CRITICAL(&levelsMux);
    memset(&latestLevels, 0, sizeof(latestLevels));
    portEXIT_CRITICAL(&levelsMux);
    s_soundLevel = 0;
    s_soundSpan = 0;
}

bool audio_input_external_active(void)
{
    return s_extActive;
}

void audio_input_feed_external(int16_t left, int16_t right)
{
    if (!s_extActive) {
        return;
    }
    if (++s_extDecim < EXT_DECIMATE) {
        return;
    }
    s_extDecim = 0;

    const int mono = ((int)left + (int)right) / 2;
    const int ac = mono >> EXT_SHIFT;
    const int acAbs = ac < 0 ? -ac : ac;

    audio_process_ac(ac);

    s_extEma += (acAbs - s_extEma) >> AUDIO_EMA_SHIFT;

    /* AGC: normalizar contra el pico reciente (decae en ~1.5 s) para que
     * los efectos usen todo el rango sin importar el nivel del stream. */
    if (s_extEma > s_extPeak) {
        s_extPeak = s_extEma;
    } else if (s_extPeak > EXT_PEAK_FLOOR) {
        s_extPeak -= ((s_extPeak - EXT_PEAK_FLOOR) >> 11) + 1;
    }
    int level = (s_extEma * EXT_LEVEL_TOP) / s_extPeak;
    if (level > 400) {
        level = 400;
    }
    s_soundLevel = level;

    s_extBuf[s_extIdx++] = (int16_t)ac;
    if (s_extIdx >= AUDIO_BLOCK_LEN) {
        audio_levels_t levels;
        compute_levels(s_extBuf, AUDIO_BLOCK_LEN, &levels);
        portENTER_CRITICAL(&levelsMux);
        latestLevels = levels;
        portEXIT_CRITICAL(&levelsMux);
        s_extIdx = 0;
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
    proc_reset();
#if AUDIO_USE_LEGACY_ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(mic_adc_channel(), ADC_ATTEN_DB_11);
#else
    analogSetPinAttenuation(PIN_MIC_ADC, ADC_11db);
    analogReadResolution(12);
#endif

    s_adcStopReq = false;
    xTaskCreatePinnedToCore(adc_task, "adc_audio", ADC_TASK_STACK, NULL, ADC_TASK_PRIO,
                            (TaskHandle_t *)&adcTaskHandle, 1);
}

void audio_input_stop(void)
{
    s_testMode = false;
    if (adcTaskHandle) {
        s_adcStopReq = true;
        /* La tarea duerme max 1 ms por ciclo: sale sola enseguida */
        for (int i = 0; i < 100 && adcTaskHandle != NULL; i++) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        s_adcStopReq = false;
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
    if (s_extActive) {
        /* PCM de radio: señal limpia, sin resta de ruido ambiente */
        return s_soundLevel;
    }
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
