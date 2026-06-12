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
    MUSIC_FX_BEAT,   /* salto de color en cada beat detectado */
    MUSIC_FX_SOLID,  /* interno (toggle Fiesta de la nube); no esta en listas */
    MUSIC_FX_COUNT
} music_fx_t;

#endif
