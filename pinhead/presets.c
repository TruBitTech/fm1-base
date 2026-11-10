/* SPDX-License-Identifier: MPL-2.0 */
#include "presets.h"

static void p(MatrixState *m, uint8_t row, uint8_t col, PinType t)
{
    if (row < MATRIX_ROWS && col < MATRIX_COLS)
        m->pins[row][col] = t;
}

/* Rows 0-15: O1s O1w O2p O2t O3p O3t Nse In1 In2 Flt Trp EnS Rng Rev StX Tape
 * Cols 0-15: Out1 Mtr Out2 Env RngA RngB Rev Filt O1F O2F O3F Dec  M  FF  Lv1 TapeIn
 */

void preset_on_the_run(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 0, PIN_WHITE);   /* O1 sine → Out1 */
    p(m, 2, 0, PIN_RED);     /* O2 pulse → Out1 */
    p(m, 4, 8, PIN_GREEN);   /* O3 LFO → O1 freq */
    p(m, 6, 0, PIN_GREEN);   /* Noise → Out1 */
}

void preset_any_colour_lead(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 0, PIN_WHITE);
    p(m, 2, 0, PIN_WHITE);
    p(m, 5, 9, PIN_RED);     /* O3 tri → O2 freq */
    p(m, 10, 0, PIN_GREEN);  /* Trap → Out1 */
}

void preset_irregular_clicks(MatrixState *m)
{
    matrix_clear(m);
    p(m, 6, 7, PIN_WHITE);   /* Noise → Filter */
    p(m, 4, 13, PIN_RED);    /* LFO → Filter freq */
    p(m, 9, 0, PIN_WHITE);   /* Filter → Out1 */
}

void preset_noise_sweep(MatrixState *m)
{
    matrix_clear(m);
    p(m, 6, 7, PIN_WHITE);   /* Noise → Filter */
    p(m, 5, 13, PIN_GREEN);  /* O3 tri → Filter freq */
    p(m, 9, 0, PIN_WHITE);   /* Filter → Out1 */
}

void preset_spring_drone(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 0, PIN_WHITE);
    p(m, 0, 6, PIN_GREEN);   /* O1 → Spring */
    p(m, 13, 0, PIN_RED);    /* Spring → Out1 */
    p(m, 5, 0, PIN_GREEN);   /* LFO tri → Out */
}

void preset_metallic_perc(MatrixState *m)
{
    matrix_clear(m);
    p(m, 0, 4, PIN_WHITE);   /* O1 → Ring A */
    p(m, 2, 5, PIN_WHITE);   /* O2 → Ring B */
    p(m, 12, 0, PIN_WHITE);  /* Ring → Out1 */
    p(m, 6, 0, PIN_RED);     /* Noise → Out */
}

void preset_empty(MatrixState *m) { matrix_clear(m); }

const Preset factory_presets[PRESET_COUNT] = {
    { "Empty Matrix",     preset_empty },
    { "On the Run",       preset_on_the_run },
    { "Any Colour Lead",  preset_any_colour_lead },
    { "Irregular Clicks", preset_irregular_clicks },
    { "Noise Sweep",      preset_noise_sweep },
    { "Spring Drone",     preset_spring_drone },
    { "Metallic Perc",    preset_metallic_perc },
};

void preset_load(MatrixState *m, int index)
{
    if (index < 0 || index >= PRESET_COUNT) index = 0;
    factory_presets[index].load(m);
}
