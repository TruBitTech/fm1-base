/* SPDX-License-Identifier: MPL-2.0 */
#ifndef PINHEAD_ENGINE_H
#define PINHEAD_ENGINE_H

#include "matrix.h"
#include "joystick.h"
#include <stdint.h>
#include <stdbool.h>

#define PH_SAMPLE_RATE 44100
#define PH_BLOCK_SIZE  64

typedef struct {
    float phase;
    float freq;
    float pw;
    float level;
} Osc;

typedef struct {
    float lp, bp, hp;
    float cutoff;
    float res;
} Filter;

typedef struct {
    float stage;
    float attack, decay, sustain, release;
    bool gate;
} Trapezoid;

typedef struct {
    float buf[2048];
    uint16_t pos;
    float mix;
    float decay;
} Spring;

typedef struct {
    float buf[4096];
    uint16_t pos;
    float time;
    float feedback;
    float mix;
} Tape;

typedef struct {
    Osc osc1, osc2, osc3;
    Filter filter;
    Trapezoid env;
    Spring spring;
    Tape tape;

    float noise_seed;
    float ring_a, ring_b;

    float reverb_mix, reverb_decay;
    float delay_mix, delay_time, delay_fb;

    JoystickState *joystick;
    MatrixState   *matrix;

    float out_l, out_r;
} Engine;

void engine_init(Engine *e, MatrixState *m, JoystickState *j);
void engine_set_gate(Engine *e, bool on);
void engine_set_note(Engine *e, float freq);
void engine_process(Engine *e, float *out_l, float *out_r, int n);
void engine_load_preset(Engine *e, int index);

#endif
