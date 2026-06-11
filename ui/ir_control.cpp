#include "ir_control.h"
#include "IRControlLite.h"
#include "config.h"
#include "lamp_log.h"
#include "music_effects.h"
#include "ui_app.h"
#include "lamp_state.h"
#include "pins.h"
#include <Arduino.h>

static IRControlLite *s_ir = nullptr;
static bool s_irStarted = false;

static void ir_commit(void)
{
    lamp_state_t *st = ui_app_get_state();
    if (!st) return;
    lamp_state_mark_dirty(st);
    ui_app_notify_state_changed();
}

static void ir_clear_audio_modes(lamp_state_t *st)
{
    st->musicFx = MUSIC_FX_NONE;
    st->musicMode = false;
}

static void ir_set_music_fx(music_fx_t fx)
{
    lamp_state_t *st = ui_app_get_state();
    if (!st) return;
    st->power = true;
    if (st->musicFx == fx && !st->musicMode) {
        ui_app_set_music_mode(true);
        ir_commit();
        return;
    }
    if (st->musicFx == fx && st->musicMode) {
        /* Mismo efecto activo en mode música: pasar a estático (sin música) */
        st->musicMode = false;
        music_effects_reset();
        ir_commit();
        return;
    }
    st->musicFx = fx;
    st->musicMode = true;
    music_effects_reset();
    ir_commit();
}

static void ir_cycle_music_fx(void)
{
    lamp_state_t *st = ui_app_get_state();
    if (!st) return;
    int next = (int)st->musicFx + 1;
    if (next >= (int)MUSIC_FX_COUNT || next <= (int)MUSIC_FX_NONE) {
        next = (int)MUSIC_FX_BAR;
    }
    st->musicFx = (music_fx_t)next;
    st->musicMode = true;
    st->power = true;
    music_effects_reset();
    ir_commit();
}

static void ir_set_color(uint16_t hue, uint8_t sat)
{
    lamp_state_t *st = ui_app_get_state();
    if (!st) return;
    const bool sameColor = (st->hue == hue && st->saturation == sat);
    const bool staticSolid = (!st->musicMode && st->musicFx == MUSIC_FX_NONE &&
                              st->effect == LAMP_EFFECT_SOLID);
    if (sameColor && staticSolid) {
        st->power = true;
        st->musicFx = MUSIC_FX_SOLID;
        ui_app_set_music_mode(true);
        ir_commit();
        return;
    }
    st->musicMode = false;
    st->musicFx = MUSIC_FX_NONE;
    st->power = true;
    st->effect = LAMP_EFFECT_SOLID;
    lamp_state_set_color_hsv(st, hue, sat);
    ir_commit();
}

static void ir_brightness_delta(int delta)
{
    if (ui_app_led_calib_active()) {
        ui_app_led_calib_step(delta);
        return;
    }
    lamp_state_t *st = ui_app_get_state();
    if (!st) return;
    int b = (int)st->brightness + delta;
    if (b < 0) b = 0;
    if (b > 255) b = 255;
    st->brightness = (uint8_t)b;
    ir_commit();
}

struct AccionesIR {
    const char *titulo;
    void (*inicio)();
    void (*presionado)();
    void (*solto)();
};

static const AccionesIR kAccionIR[] = {
    {"Brillo+", [] { ir_brightness_delta(+20); }, [] { ir_brightness_delta(+10); }, nullptr},
    {"Brillo-", [] { ir_brightness_delta(-20); }, [] { ir_brightness_delta(-10); }, nullptr},
    {"ON", [] {
         lamp_state_t *st = ui_app_get_state();
         if (st) { ir_clear_audio_modes(st); st->power = true; ir_commit(); }
     }, nullptr, nullptr},
    {"OFF", [] {
         lamp_state_t *st = ui_app_get_state();
         if (st) { st->power = false; ir_commit(); }
     }, nullptr, nullptr},

    {"R", [] { ir_set_color(0, 255); }, nullptr, nullptr},
    {"G", [] { ir_set_color(85, 255); }, nullptr, nullptr},
    {"B", [] { ir_set_color(170, 255); }, nullptr, nullptr},
    {"BLANCO", [] { ir_set_color(0, 0); }, nullptr, nullptr},

    {"NARANJA", [] { ir_set_color(20, 255); }, nullptr, nullptr},
    {"AMARILLO", [] { ir_set_color(45, 255); }, nullptr, nullptr},
    {"CYAN", [] { ir_set_color(128, 255); }, nullptr, nullptr},
    {"PURPURA", [] { ir_set_color(200, 255); }, nullptr, nullptr},

    {"Arcoiris", [] {
         lamp_state_t *st = ui_app_get_state();
         if (st) { ir_clear_audio_modes(st); st->power = true; st->effect = LAMP_EFFECT_RAINBOW; ir_commit(); }
     }, nullptr, nullptr},
    {"Persecucion", [] { ir_set_music_fx(MUSIC_FX_CHASE); }, nullptr, nullptr},
    {"Colombia", [] { ir_set_music_fx(MUSIC_FX_WAVE); }, nullptr, nullptr},
    {"Respiracion", [] { ir_set_music_fx(MUSIC_FX_BREATH); }, nullptr, nullptr},

    {"Fiesta", [] { ir_set_music_fx(MUSIC_FX_PARTY); }, nullptr, nullptr},
    {"M2 Barra", [] { ir_set_music_fx(MUSIC_FX_BAR); }, nullptr, nullptr},
    {"Estrobo", [] { ir_set_music_fx(MUSIC_FX_STROBE); }, nullptr, nullptr},
    {"Efecto+", [] { ir_cycle_music_fx(); }, nullptr, nullptr},
};

static const int kNumAccionesIR = (int)(sizeof(kAccionIR) / sizeof(kAccionIR[0]));

void IRControlLite_inicio(int codigoIR)
{
    if (codigoIR < 0 || codigoIR >= kNumAccionesIR) return;
#if IR_DEBUG_LOG
    LAMP_LOG("IR accion %d: %s\n", codigoIR, kAccionIR[codigoIR].titulo);
#endif
    if (kAccionIR[codigoIR].inicio) {
        kAccionIR[codigoIR].inicio();
    }
}

void IRControlLite_presionado(int codigoIR)
{
    if (codigoIR < 0 || codigoIR >= kNumAccionesIR) return;
    if (kAccionIR[codigoIR].presionado) {
        kAccionIR[codigoIR].presionado();
    }
}

void IRControlLite_solto(int codigoIR)
{
    if (codigoIR < 0 || codigoIR >= kNumAccionesIR) return;
    if (kAccionIR[codigoIR].solto) {
        kAccionIR[codigoIR].solto();
    }
}

static void ir_control_start(void)
{
    if (s_irStarted) return;
    s_ir = new IRControlLite(PIN_IR);
    if (!s_ir->iniciar()) {
        LAMP_LOG("IR: fallo init GPIO %d\n", PIN_IR);
        delete s_ir;
        s_ir = nullptr;
        return;
    }
    s_irStarted = true;
}

void ir_control_service(void)
{
    if (!ui_app_ir_allowed()) {
        return;
    }

    if (!s_irStarted) {
        ir_control_start();
    }

    if (s_ir) {
        s_ir->procesar();
    }
}
