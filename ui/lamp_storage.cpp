#include "lamp_storage.h"
#include "music_fx.h"
#include "ui_control_screen.h"
#include <Preferences.h>
#include <Arduino.h>

#define LAMP_STORE_NS     "lampv3"
#define LAMP_STORE_MAGIC  0x4C503033U
#define LAMP_STORE_VER    2U

static bool clamp_state(lamp_state_t *state)
{
    if (!state) {
        return false;
    }
    if (state->effect >= LAMP_EFFECT_COUNT) {
        state->effect = LAMP_EFFECT_SOLID;
    }
    if (state->musicFx >= MUSIC_FX_COUNT) {
        state->musicFx = MUSIC_FX_NONE;
    }
    return true;
}

static void clamp_ui(lamp_ui_persist_t *ui)
{
    if (!ui) {
        return;
    }
    if (ui->activeTab > UI_TAB_SETTINGS) {
        ui->activeTab = UI_TAB_COLOR;
    }
    if (ui->effectSpeed < 10U) {
        ui->effectSpeed = 10U;
    }
    if (ui->micSensPct > 100U) {
        ui->micSensPct = 50U;
    }
    if (ui->timerPresetIdx > 4U) {
        ui->timerPresetIdx = 0U;
    }
}

bool lamp_storage_load(lamp_state_t *state, lamp_ui_persist_t *ui)
{
    if (!state || !ui) {
        return false;
    }

    Preferences prefs;
    if (!prefs.begin(LAMP_STORE_NS, true)) {
        return false;
    }

    const uint32_t magic = prefs.getUInt("magic", 0U);
    const uint32_t ver = prefs.getUInt("ver", 0U);
    if (magic != LAMP_STORE_MAGIC || (ver != LAMP_STORE_VER && ver != 1U)) {
        prefs.end();
        return false;
    }

    state->power = prefs.getBool("power", true);
    state->brightness = (uint8_t)prefs.getUChar("bright", 128U);
    state->hue = (uint16_t)prefs.getUShort("hue", 0U);
    state->saturation = (uint8_t)prefs.getUChar("sat", 255U);
    state->effect = (lamp_effect_t)prefs.getUChar("effect", (uint8_t)LAMP_EFFECT_SOLID);
    state->musicFx = (music_fx_t)prefs.getUChar("musicFx", (uint8_t)MUSIC_FX_NONE);
    if (ver >= 2U) {
        state->musicMode = prefs.getBool("musicMode", false);
    } else {
        state->musicMode = (state->musicFx != MUSIC_FX_NONE);
    }
    state->dirty = true;

    ui->activeTab = (uint8_t)prefs.getUChar("tab", UI_TAB_COLOR);
    ui->effectSpeed = (uint16_t)prefs.getUShort("speed", 200U);
    ui->effectReverse = prefs.getBool("reverse", false);
    ui->micSensPct = (uint8_t)prefs.getUChar("micPct", 50U);
    ui->nightMode = prefs.getBool("night", false);
    ui->timerPresetIdx = (uint8_t)prefs.getUChar("timerIdx", 0U);

    prefs.end();
    clamp_state(state);
    clamp_ui(ui);
    return true;
}

void lamp_storage_save(const lamp_state_t *state, const lamp_ui_persist_t *ui)
{
    if (!state || !ui) {
        return;
    }

    Preferences prefs;
    if (!prefs.begin(LAMP_STORE_NS, false)) {
        return;
    }

    prefs.putUInt("magic", LAMP_STORE_MAGIC);
    prefs.putUInt("ver", LAMP_STORE_VER);
    prefs.putBool("power", state->power);
    prefs.putUChar("bright", state->brightness);
    prefs.putUShort("hue", state->hue);
    prefs.putUChar("sat", state->saturation);
    prefs.putUChar("effect", (uint8_t)state->effect);
    prefs.putUChar("musicFx", (uint8_t)state->musicFx);
    prefs.putBool("musicMode", state->musicMode);
    prefs.putUChar("tab", ui->activeTab);
    prefs.putUShort("speed", ui->effectSpeed);
    prefs.putBool("reverse", ui->effectReverse);
    prefs.putUChar("micPct", ui->micSensPct);
    prefs.putBool("night", ui->nightMode);
    prefs.putUChar("timerIdx", ui->timerPresetIdx);
    prefs.end();
}
