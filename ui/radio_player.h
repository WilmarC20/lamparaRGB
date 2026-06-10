#ifndef RADIO_PLAYER_H
#define RADIO_PLAYER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void radio_player_init(void);
void radio_player_loop(void);
void radio_player_service(void);
bool radio_player_is_playing(void);
bool radio_player_is_busy(void);
int radio_player_get_station(void);
bool radio_player_play(uint8_t station_idx);
void radio_player_stop(void);
void radio_player_toggle(uint8_t station_idx);
const char *radio_player_station_name(uint8_t idx);
uint8_t radio_player_station_count(void);
const char *radio_player_status_text(void);
void radio_player_set_volume(uint8_t pct);
uint8_t radio_player_get_volume(void);

#ifdef __cplusplus
}
#endif

#endif
