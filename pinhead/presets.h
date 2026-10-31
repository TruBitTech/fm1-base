/* SPDX-License-Identifier: MPL-2.0 */
#ifndef PINHEAD_PRESETS_H
#define PINHEAD_PRESETS_H

#include "matrix.h"

typedef struct {
    const char *name;
    void (*load)(MatrixState *m);
} Preset;

void preset_on_the_run(MatrixState *m);
void preset_any_colour_lead(MatrixState *m);
void preset_irregular_clicks(MatrixState *m);
void preset_noise_sweep(MatrixState *m);
void preset_spring_drone(MatrixState *m);
void preset_metallic_perc(MatrixState *m);
void preset_empty(MatrixState *m);

#define PRESET_COUNT 7
extern const Preset factory_presets[PRESET_COUNT];

void preset_load(MatrixState *m, int index);

#endif
