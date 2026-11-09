/* SPDX-License-Identifier: MPL-2.0 */
#include "presets.h"

static void p(MatrixState *m, uint8_t row, uint8_t col, PinType t)
{
    if (row < MATRIX_ROWS && col < MATRIX_COLS)
        m->pins[row][col] = t;
}

/* Row sources: 0=Osc1, 1=Osc2, 2=Osc3, 5=Noise, 8=Env, 9=Ring */
/* Col destinations: 3=Filter, 9=Output */

void preset_on_the_run(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 9, PIN_WHITE);   /* osc1 → out */
    p(m, 1, 9, PIN_RED);     /* osc2 → out */
    p(m, 2, 3, PIN_WHITE);   /* osc3 → filter */
    p(m, 5, 9, PIN_GREEN);   /* noise → out */
    p(m, 9, 9, PIN_WHITE);   /* ring → out */
}

void preset_any_colour_lead(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 9, PIN_WHITE);
    p(m, 1, 9, PIN_WHITE);
    p(m, 2, 9, PIN_RED);
    p(m, 8, 9, PIN_GREEN);   /* env → out */
    p(m, 5, 3, PIN_GREEN);
}

void preset_irregular_clicks(MatrixState *m)
{
    matrix_clear(m);
    p(m, 5, 9, PIN_WHITE);   /* noise → out */
    p(m, 0, 3, PIN_RED);
    p(m, 1, 9, PIN_GREEN);
    p(m, 9, 9, PIN_WHITE);
}

void preset_noise_sweep(MatrixState *m)
{
    matrix_clear(m);
    p(m, 5, 9, PIN_WHITE);
    p(m, 5, 3, PIN_GREEN);
    p(m, 0, 9, PIN_RED);
    p(m, 8, 9, PIN_WHITE);
}

void preset_spring_drone(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 9, PIN_WHITE);
    p(m, 2, 9, PIN_GREEN);
    p(m, 5, 9, PIN_WHITE);
    p(m, 9, 9, PIN_RED);
    p(m, 8, 3, PIN_GREEN);
}

void preset_metallic_perc(MatrixState *m)
{
    matrix_clear(m);
    p(m, 1, 9, PIN_WHITE);
    p(m, 5, 9, PIN_WHITE);
    p(m, 9, 9, PIN_RED);
    p(m, 0, 3, PIN_GREEN);
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
