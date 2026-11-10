/* SPDX-License-Identifier: MPL-2.0
 * Pin Head engine — Synthi-style pin matrix with tape mod
 *
 * Rows (sources, 0-based):
 *  0 O1 sine   1 O1 saw    2 O2 pulse  3 O2 tri
 *  4 O3 pulse  5 O3 tri    6 Noise     7 In1
 *  8 In2       9 Filter    10 Trap     11 Env sig
 * 12 Ring      13 Spring   14 Stick X  15 Tape out
 *
 * Cols (destinations):
 *  A Out1  B Meter  C Out2  D Env
 *  E RingA F RingB  G Rev   H Filt in
 *  I O1F   J O2F    K O3F   L Decay
 *  M (spare) N FiltF O Lv1  P Tape in
 */
#include "engine.h"
#include "presets.h"
#include "fastmath.h"

static float frand(float *seed)
{
    *seed = fm_sinf(*seed * 12.9898f) * 43758.5453f;
    return (*seed - (int)(*seed)) * 2.0f - 1.0f;
}

static float eclamp(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float wave_pulse(float phase, float pw)
{
    float edge = 0.03f;
    if (phase < edge)
        return -1.0f + phase / edge * 2.0f;
    if (phase < pw)
        return 1.0f;
    if (phase < pw + edge)
        return 1.0f - (phase - pw) / edge * 2.0f;
    return -1.0f;
}

static float wave_saw(float phase)
{
    return phase * 2.0f - 1.0f;
}

static float wave_tri(float phase)
{
    float t = phase * 2.0f;
    if (t > 1.0f) t = 2.0f - t;
    return t * 2.0f - 1.0f;
}

static float wave_sine(float phase)
{
    return fm_sinf(phase * 6.2831853f);
}

static float osc_tick(Osc *o, float sr, float wave)
{
    o->phase += o->freq / sr;
    if (o->phase >= 1.0f) o->phase -= 1.0f;
    float s;
    if (wave == 0.0f) s = wave_sine(o->phase);
    else if (wave == 1.0f) s = wave_saw(o->phase);
    else if (wave == 2.0f) s = wave_pulse(o->phase, o->pw);
    else s = wave_tri(o->phase);
    return s * o->level;
}

static void filter_tick(Filter *f, float in, float sr)
{
    float f0 = f->cutoff / sr;
    if (f0 > 0.45f) f0 = 0.45f;
    if (f0 < 0.001f) f0 = 0.001f;
    float q = 1.0f - f->res * 0.85f;
    f->lp += f0 * (in - f->lp - f->bp * q);
    f->bp += f0 * f->lp;
    f->hp = in - f->lp - f->bp;
}

static float env_tick(Trapezoid *e, float sr)
{
    float rate = e->gate ? e->attack : e->release;
    if (rate < 0.001f) rate = 0.001f;
    float step = 1.0f / (rate * sr);
    if (e->gate) {
        e->stage += step;
        if (e->stage > 1.0f) e->stage = 1.0f;
    } else {
        e->stage -= step;
        if (e->stage < 0.0f) e->stage = 0.0f;
    }
    return e->stage;
}

static float spring_tick(Spring *s, float in)
{
    float fb = s->buf[s->pos];
    s->buf[s->pos] = in * 0.35f + fb * s->decay;
    s->pos = (s->pos + 1) % 2048;
    return fb;
}

static float tape_tick(Tape *t, float in, float sr)
{
    int delay = (int)(t->time * sr);
    if (delay < 1) delay = 1;
    if (delay > 4095) delay = 4095;
    int rpos = (t->pos + 4096 - delay) & 4095;
    float fb = t->buf[rpos];
    float wet = fb * t->feedback + in;
    if (wet > 0.9f) wet = 0.9f;
    if (wet < -0.9f) wet = -0.9f;
    t->buf[t->pos] = wet;
    t->pos = (t->pos + 1) & 4095;
    return fb;
}

static void clear_bytes(void *p, unsigned n)
{
    unsigned char *b = (unsigned char *)p;
    while (n--) *b++ = 0;
}

void engine_init(Engine *e, MatrixState *m, JoystickState *j)
{
    clear_bytes(e, sizeof(*e));
    e->matrix = m;
    e->joystick = j;

    e->osc1.freq = 220.0f; e->osc1.pw = 0.5f; e->osc1.level = 0.5f;
    e->osc2.freq = 330.0f; e->osc2.pw = 0.4f; e->osc2.level = 0.4f;
    e->osc3.freq = 1.2f;   e->osc3.pw = 0.5f; e->osc3.level = 0.8f;

    e->filter.cutoff = 1800.0f;
    e->filter.res = 0.3f;

    e->env.attack = 0.01f;
    e->env.decay = 0.2f;
    e->env.sustain = 0.6f;
    e->env.release = 0.3f;

    e->spring.mix = 0.3f;
    e->spring.decay = 0.7f;

    e->tape.time = 0.25f;
    e->tape.feedback = 0.35f;
    e->tape.mix = 0.5f;

    e->reverb_mix = 0.0f;
    e->reverb_decay = 0.5f;
    e->delay_mix = 0.0f;
    e->delay_time = 0.3f;
    e->delay_fb = 0.3f;

    e->noise_seed = 0.123f;
}

void engine_set_gate(Engine *e, bool on) { e->env.gate = on; }

void engine_set_note(Engine *e, float freq)
{
    e->osc1.freq = freq;
    e->osc2.freq = freq * 1.5f;
}

void engine_load_preset(Engine *e, int index)
{
    if (e->matrix) preset_load(e->matrix, index);
}

static float filt_out, ring_out, tape_out, spring_out;

static void evaluate_matrix(Engine *e, float *dest)
{
    const MatrixState *m = e->matrix;
    if (!m) return;

    float sr = (float)PH_SAMPLE_RATE;

    float f1 = e->osc1.freq;
    float f2 = e->osc2.freq;
    float f3 = e->osc3.freq;

    float sources[16];
    sources[0] = osc_tick(&e->osc1, sr, 0.0f);
    sources[1] = wave_saw(e->osc1.phase) * e->osc1.level;
    sources[2] = osc_tick(&e->osc2, sr, 2.0f);
    sources[3] = wave_tri(e->osc2.phase) * e->osc2.level;
    sources[4] = osc_tick(&e->osc3, sr, 2.0f);
    sources[5] = wave_tri(e->osc3.phase) * e->osc3.level;
    sources[6] = frand(&e->noise_seed) * 0.25f;
    sources[7] = 0.0f;
    sources[8] = 0.0f;
    sources[9] = filt_out;
    sources[10] = env_tick(&e->env, sr);
    sources[11] = sources[10];
    sources[12] = ring_out;
    sources[13] = spring_out;
    sources[14] = e->joystick ? joystick_mod_x(e->joystick) : 0.0f;
    sources[15] = tape_out;

    for (int i = 0; i < 16; i++) dest[i] = 0.0f;

    for (int r = 0; r < 16; r++) {
        for (int c = 0; c < 16; c++) {
            PinType pt = matrix_get(m, r, c);
            if (pt == PIN_NONE) continue;
            dest[c] += sources[r] * pin_gain(pt);
        }
    }

    e->osc1.freq = eclamp(f1 + dest[8] * 80.0f, 20.0f, 8000.0f);
    e->osc2.freq = eclamp(f2 + dest[9] * 80.0f, 20.0f, 8000.0f);
    e->osc3.freq = eclamp(f3 + dest[10] * 4.0f, 0.1f, 40.0f);

    float fin = dest[7];
    float base_cut = 1200.0f;
    e->filter.cutoff = eclamp(base_cut + dest[13] * 2500.0f, 80.0f, 8000.0f);
    filter_tick(&e->filter, fin, sr);
    filt_out = e->filter.lp;

    ring_out = dest[4] * dest[5];
    tape_out = tape_tick(&e->tape, dest[15], sr);
    spring_out = spring_tick(&e->spring, dest[6]);
}

void engine_process(Engine *e, float *out_l, float *out_r, int n)
{
    for (int i = 0; i < n; i++) {
        float dest[16];
        evaluate_matrix(e, dest);

        float dry = dest[0] + dest[2] * 0.8f;

        if (e->reverb_mix > 0.01f)
            dry = dry * (1.0f - e->reverb_mix) + spring_out * e->reverb_mix;

        if (dry > 0.8f) dry = 0.8f;
        if (dry < -0.8f) dry = -0.8f;

        float jx = e->joystick ? joystick_mod_x(e->joystick) : 0.0f;
        *out_l++ = dry * (1.0f + jx * 0.1f);
        *out_r++ = dry * (1.0f - jx * 0.1f);
    }
}
