#ifndef MUSIC_FX_H
#define MUSIC_FX_H

typedef enum {
    MUSIC_FX_NONE = 0,
    MUSIC_FX_BAR,
    MUSIC_FX_PARTY,
    MUSIC_FX_CHASE,
    MUSIC_FX_WAVE,   /* slot reutilizado: efecto "Musica Colombia" (bandera) */
    MUSIC_FX_BREATH,
    MUSIC_FX_STROBE,
    MUSIC_FX_SOLID,
    MUSIC_FX_COUNT
} music_fx_t;

#endif
