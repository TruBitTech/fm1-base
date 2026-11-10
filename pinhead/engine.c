/* SPDX-License-Identifier: MPL-2.0 */
#include "engine.h"
#include "presets.h"
#include "fastmath.h"

static float frand(float *seed)
{
    *seed = fm_sinf(*seed * 12.9898f) * 43758.5453f;
    return (*seed - (int)(*seed)) * 2.0f - 1.0f;
}

static float osc_tick(Osc *o, float sr)
{
    o->phase += o->freq / sr;
    if (o->phase >= 1.0f) o->phase -= 1.0f;
    float s;
    if (o->phase < 0.04f)
        s = -1.0f + o->phase * 50.0f;
    else if (o->phase < o->pw)
        s = 1.0f;
    else if (o->phase < o->pw + 0.04f)
        s = 1.0f - (o->phase - o->pw) * 50.0f;
    else
        s = -1.0f;
    return s * o->level;
}

static void filter_tick(Filter *f, float in, float sr)
{
    float f0 = f->cutoff / sr;
    if (f0 > 0.45f) f0 = 0.45f;
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
    return in * (1.0f - s->mix) + fb * s->mix;
}

static float tape_tick(Tape *t, float in, float sr)
{
    int delay = (int)(t->time * sr);
    if (delay < 1) delay = 1;
    if (delay > 4095) delay = 4095;
    int rpos = (t->pos + 4096 - delay) & 4095;
    float fb = t->buf[rpos];
    t->buf[t->pos] = in + fb * t->feedback;
    t->pos = (t->pos + 1) & 4095;
    return in * (1.0f - t->mix) + fb * t->mix;
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

    e->osc1.freq = 110.0f; e->osc1.pw = 0.5f; e->osc1.level = 0.5f;
    e->osc2.freq = 220.0f; e->osc2.pw = 0.5f; e->osc2.level = 0.35f;
    e->osc3.freq = 1.5f;   e->osc3.pw = 0.5f; e->osc3.level = 0.4f; /* LFO range */

    e->filter.cutoff = 1800.0f;
    e->filter.res = 0.35f;

    e->env.attack = 0.005f;
    e->env.decay = 0.15f;
    e->env.sustain = 0.7f;
    e->env.release = 0.25f;

    e->spring.mix = 0.25f;
    e->spring.decay = 0.65f;

    e->tape.time = 0.22f;
    e->tape.feedback = 0.35f;
    e->tape.mix = 0.18f;

    e->reverb_mix = 0.15f;
    e->reverb_decay = 0.55f;
    e->delay_mix = 0.12f;
    e->delay_time = 0.28f;
    e->delay_fb = 0.3f;

    e->noise_seed = 0.123f;
}

void engine_set_gate(Engine *e, bool on) { e->env.gate = on; }

void engine_set_note(Engine *e, float freq)
{
    e->osc1.freq = freq;
    e->osc2.freq = freq * 1.5f;
    /* osc3 stays as LFO unless matrix routes a pitch control */
}

void engine_load_preset(Engine *e, int index)
{
    if (e->matrix) preset_load(e->matrix, index);
}

static void evaluate_matrix(Engine *e, float *dest)
{
    const MatrixState *m = e->matrix;
    if (!m) return;

    float sources[16] = {0};
    /* Authentic row mapping */
    sources[0] = osc_tick(&e->osc1, PH_SAMPLE_RATE);          /* 1 Osc1 sine */
    sources[1] = osc_tick(&e->osc1, PH_SAMPLE_RATE) * 0.8f;   /* 2 Osc1 saw (approx) */
    sources[2] = osc_tick(&e->osc2, PH_SAMPLE_RATE);          /* 3 Osc2 pulse */
    sources[3] = osc_tick(&e->osc2, PH_SAMPLE_RATE) * 0.7f;   /* 4 Osc2 tri (approx) */
    sources[4] = osc_tick(&e->osc3, PH_SAMPLE_RATE);          /* 5 Osc3 pulse (LFO) */
    sources[5] = osc_tick(&e->osc3, PH_SAMPLE_RATE) * 0.6f;   /* 6 Osc3 tri (LFO) */
    sources[6] = frand(&e->noise_seed) * 0.2f;                /* 7 Noise */
    sources[7] = 0.0f;                                        /* 8 Input Ch1 */
    sources[8] = 0.0f;                                        /* 9 Input Ch2 */
    sources[9] = 0.0f;                                        /* 10 Filter (filled after) */
    sources[10] = env_tick(&e->env, PH_SAMPLE_RATE);          /* 11 Trapezoid */
    sources[11] = sources[10] * 0.8f;                         /* 12 Env signal */
    sources[12] = sources[0] * sources[2];                    /* 13 Ring mod */
    sources[13] = 0.0f;                                       /* 14 Reverb (post) */
    sources[14] = e->joystick ? joystick_mod_x(e->joystick) : 0.0f; /* 15 Stick X */
    sources[15] = e->joystick ? joystick_mod_y(e->joystick) : 0.0f; /* 16 Stick Y */

    for (int i = 0; i < 16; i++) dest[i] = 0.0f;

    for (int r = 0; r < 16; r++) {
        for (int c = 0; c < 16; c++) {
            PinType pt = matrix_get(m, r, c);
            if (pt == PIN_NONE) continue;
            float g = pin_gain(pt);
            dest[c] += sources[r] * g;
        }
    }

    /* Column H (7) = Filter signal in, Column N (13) = Filter freq control */
    float fin = dest[7];
    float fmod = dest[13];
    if (fmod != 0.0f)
        e->filter.cutoff = 200.0f + fmod * 2000.0f;
    filter_tick(&e->filter, fin, PH_SAMPLE_RATE);
    dest[9] = e->filter.lp;   /* row 10 = Filter output */
    sources[9] = dest[9];

    /* LFO modulation: if Osc3 is routed to Osc freq columns (I/J/K = 8/9/10) */
    if (dest[8] != 0.0f) e->osc1.freq += dest[8] * 50.0f;
    if (dest[9] != 0.0f && dest[7] == 0.0f) e->osc2.freq += dest[9] * 50.0f;
    if (dest[10] != 0.0f) e->osc3.freq += dest[10] * 2.0f;

    /* Fallback if nothing routed to output columns A or C (0 or 2) */
    if (dest[0] == 0.0f && dest[2] == 0.0f)
        dest[0] = sources[0] * 0.4f + sources[2] * 0.3f;
}

void engine_process(Engine *e, float *out_l, float *out_r, int n)
{
    for (int i = 0; i < n; i++) {
        float dest[16];
        evaluate_matrix(e, dest);

        float dry = dest[0]; /* Output Ch 1 */
        float wet = spring_tick(&e->spring, dry);
        float tape = tape_tick(&e->tape, wet, PH_SAMPLE_RATE);

        float mix = dry * 0.65f + tape * 0.35f;
        if (e->reverb_mix > 0.01f)
            mix = mix * (1.0f - e->reverb_mix) + wet * e->reverb_mix;

        if (mix > 0.75f) mix = 0.75f;
        if (mix < -0.75f) mix = -0.75f;

        float jx = e->joystick ? joystick_mod_x(e->joystick) : 0.0f;

        *out_l++ = mix * (1.0f + jx * 0.15f);
        *out_r++ = mix * (1.0f - jx * 0.15f);
    }
}
