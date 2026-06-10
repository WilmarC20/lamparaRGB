#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void audio_output_init(void);
bool audio_output_is_playing(void);
void audio_output_play_test(void);

#ifdef __cplusplus
}
#endif

#endif
