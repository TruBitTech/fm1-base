/* SPDX-License-Identifier: MPL-2.0 */
#include "presets.h"

static void p(MatrixState *m, uint8_t row, uint8_t col, PinType t)
{
    if (row < MATRIX_ROWS && col < MATRIX_COLS)
        m->pins[row][col] = t;
}

/* Rows: 0=O1sin, 1=O1saw, 2=O2pls, 3=O2tri, 4=O3pls(LFO), 5=O3tri(LFO), 6=Noise, 10=Filt, 11=Trap, 12=EnvS, 13=Ring */
/* Cols: 0=Out1, 7=Filt signal, 8=Osc1F, 9=Osc2F, 10=Osc3F, 13=FiltF */

void preset_on_the_run(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 0, PIN_WHITE);   /* O1 → Out1 */
    p(m, 2, 0, PIN_RED);     /* O2 → Out1 */
    p(m, 4, 8, PIN_GREEN);   /* LFO → Osc1 freq */
    p(m, 6, 0, PIN_GREEN);   /* Noise → Out1 */
}

void preset_any_colour_lead(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 0, PIN_WHITE);
    p(m, 2, 0, PIN_WHITE);
    p(m, 4, 9, PIN_RED);     /* LFO → Osc2 freq */
    p(m, 11, 0, PIN_GREEN);  /* Trap → Out1 */
}

void preset_irregular_clicks(MatrixState *m)
{
    matrix_clear(m);
    p(m, 6, 0, PIN_WHITE);   /* Noise → Out */
    p(m, 4, 8, PIN_RED);
    p(m, 0, 7, PIN_GREEN);   /* O1 → Filter */
    p(m, 10, 0, PIN_WHITE);  /* Filter → Out */
}

void preset_noise_sweep(MatrixState *m)
{
    matrix_clear(m);
    p(m, 6, 7, PIN_WHITE);   /* Noise → Filter */
    p(m, 4, 13, PIN_GREEN);  /* LFO → Filter freq */
    p(m, 10, 0, PIN_WHITE);  /* Filter → Out */
}

void preset_spring_drone(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 0, PIN_WHITE);
    p(m, 5, 0, PIN_GREEN);   /* LFO tri → Out */
    p(m, 6, 0, PIN_WHITE);
    p(m, 13, 0, PIN_RED);    /* Ring → Out */
}

void preset_metallic_perc(MatrixState *m)
{
    matrix_clear(m);
    p(m, 2, 0, PIN_WHITE);
    p(m, 6, 0, PIN_WHITE);
    p(m, 13, 0, PIN_RED);
    p(m, 4, 13, PIN_GREEN);  /* LFO → Filter freq */
}

void preset_empty(MatrixState *m) { matrix_clear(m); }

const Preset factory_presets[PRESET_COUNT] = {
    { "On the Run",       preset_on_the_run },
    { "Any Colour Lead",  preset_any_colour_lead },
    { "Irregular Clicks", preset_irregular_clicks },
    { "Noise Sweep",      preset_noise_sweep },
    { "Spring Drone",     preset_spring_drone },
    { "Metallic Perc",    preset_metallic_perc },
    { "Empty Matrix",     preset_empty },
};

void preset_load(MatrixState *m, int index)
{
    if (index < 0 || index >= PRESET_COUNT) index = 0;
    factory_presets[index].load(m);
}
