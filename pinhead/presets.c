/* SPDX-License-Identifier: MPL-2.0 */
#include "presets.h"

static void p(MatrixState *m, uint8_t row, uint8_t col, PinType t)
{
    if (row < MATRIX_ROWS && col < MATRIX_COLS)
        m->pins[row][col] = t;
}

void preset_on_the_run(MatrixState *m)
{
    matrix_clear(m);
    p(m, 6, 1, PIN_RED);
    p(m, 2, 3, PIN_WHITE);
    p(m, 5, 6, PIN_WHITE);
    p(m, 8, 7, PIN_WHITE);
    p(m, 9, 9, PIN_WHITE);
    p(m, 8, 4, PIN_GREEN);
    p(m, 9, 8, PIN_GREEN);
}

void preset_any_colour_lead(MatrixState *m)
{
    matrix_clear(m);
    p(m, 7, 0, PIN_RED);
    p(m, 7, 1, PIN_RED);
    p(m, 0, 3, PIN_WHITE);
    p(m, 2, 3, PIN_WHITE);
    p(m, 9, 9, PIN_WHITE);
    p(m, 8, 4, PIN_WHITE);
    p(m, 0, 6, PIN_GREEN);
    p(m, 2, 7, PIN_GREEN);
}

void preset_irregular_clicks(MatrixState *m)
{
    matrix_clear(m);
    p(m, 1, 0, PIN_WHITE);
    p(m, 4, 1, PIN_WHITE);
    p(m, 5, 3, PIN_WHITE);
    p(m, 1, 4, PIN_GREEN);
    p(m, 9, 9, PIN_WHITE);
}

void preset_noise_sweep(MatrixState *m)
{
    matrix_clear(m);
    p(m, 5, 3, PIN_WHITE);
    p(m, 8, 4, PIN_WHITE);
    p(m, 8, 5, PIN_GREEN);
    p(m, 9, 9, PIN_WHITE);
    p(m, 9, 8, PIN_GREEN);
}

void preset_spring_drone(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 3, PIN_WHITE);
    p(m, 2, 3, PIN_GREEN);
    p(m, 8, 4, PIN_GREEN);
    p(m, 9, 8, PIN_WHITE);
    p(m, 15, 3, PIN_GREEN);
    p(m, 9, 9, PIN_WHITE);
}

void preset_metallic_perc(MatrixState *m)
{
    matrix_clear(m);
    p(m, 5, 6, PIN_WHITE);
    p(m, 8, 7, PIN_WHITE);
    p(m, 9, 9, PIN_WHITE);
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
