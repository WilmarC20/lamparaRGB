#ifndef LAMP_STORAGE_H
#define LAMP_STORAGE_H

#include "lamp_state.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t activeTab;
    uint16_t effectSpeed;
    bool effectReverse;
    uint8_t micSensPct;
    bool nightMode;
    uint8_t timerPresetIdx;
    uint8_t musicPalette;   /* 0=Arcoiris 1=Tropical 2=Rock 3=Chill */
    uint8_t sunriseIdx;     /* 0=Off, 1..4 = hora de amanecer */
} lamp_ui_persist_t;

bool lamp_storage_load(lamp_state_t *state, lamp_ui_persist_t *ui);
void lamp_storage_save(const lamp_state_t *state, const lamp_ui_persist_t *ui);

#ifdef __cplusplus
}
#endif

#endif
