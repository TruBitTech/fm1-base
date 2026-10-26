/* SPDX-License-Identifier: MPL-2.0
 * Pin Head application — matrix-centric UI on fm1-base
 */
#include "app.h"

/* ------------------------------------------------------------------ state --- */
enum { MODE_MATRIX, MODE_PLAY, MODE_GLO };
static uint8_t mode = MODE_MATRIX;
static uint8_t cursor_x = 0, cursor_y = 0;   /* 0..15 for full, or 0..9 for core view */
static uint8_t full_view = 0;                /* 0 = 10x10 core (wrap), 1 = 16x16 full */
static uint8_t pin[16][16];                  /* 0=none, 1=white, 2=red, 3=green */
static uint8_t glo_page = 0;

/* ------------------------------------------------------------------ audio --- */
static volatile int32_t voice_note = -1;
static float phase;

void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    (void)master_q12;
    uint32_t i;
    float inc = voice_note >= 0 ? 440.0f * powf(2.0f, (voice_note - 69) / 12.0f) / 44100.0f : 0.0f;
    for (i = 0; i < n; i++) {
        float s = 0.0f;
        if (inc) {
            phase += inc;
            if (phase >= 1.0f) phase -= 1.0f;
            s = sinf(phase * 6.2831853f) * 0.2f;
        }
        lr[2 * i] = lr[2 * i + 1] = (int32_t)(s * 8000000.0f);
    }
}

void app_panic(void) { voice_note = -1; }

void app_init(void)
{
    /* clear matrix */
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            pin[y][x] = 0;
}

/* ------------------------------------------------------------------ UI --- */
static void draw_matrix(void)
{
    /* simple 10x10 or 16x16 grid of pin dots */
    int size = full_view ? 16 : 10;
    int cell = full_view ? 12 : 18;
    cv_begin(240, 200, C_BLACK);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int px = 10 + x * cell;
            int py = 20 + y * cell;
            uint8_t p = pin[y][x];
            uint16_t col = C_DIM;
            if (p == 1) col = C_WHITE;
            else if (p == 2) col = C_RED;
            else if (p == 3) col = C_GREEN;
            cv_rect(px - 2, py - 2, 4, 4, col);
            if (x == cursor_x && y == cursor_y)
                cv_rect(px - 4, py - 4, 8, 8, C_HI);
        }
    }
    cv_text(4, 4, &FONT_XS, full_view ? "FULL" : "CORE", C_GRAY);
    cv_blit(0, 0);
}

static void input(void)
{
    uint32_t btn = plat_buttons();
    int32_t e;

    /* SELECT cycles view / mode for now */
    if ((e = plat_enc(EN_SELECT))) {
        if (mode == MODE_MATRIX)
            full_view = !full_view;
        else if (mode == MODE_GLO)
            glo_page = (glo_page + e) & 3;
    }

    /* K1/K2 move cursor in matrix mode */
    if (mode == MODE_MATRIX) {
        if ((e = plat_enc(EN_K1))) {
            cursor_x = (uint8_t)((cursor_x + e + (full_view ? 16 : 10)) % (full_view ? 16 : 10));
        }
        if ((e = plat_enc(EN_K2))) {
            cursor_y = (uint8_t)((cursor_y + e + (full_view ? 16 : 10)) % (full_view ? 16 : 10));
        }
        /* K3 cycles pin */
        if ((e = plat_enc(EN_K3)) && e) {
            pin[cursor_y][cursor_x] = (pin[cursor_y][cursor_x] + 1) & 3;
        }
    }

    /* GLO button toggles mode */
    static uint32_t last_btn;
    if ((btn & (1u << B_GLO)) && !(last_btn & (1u << B_GLO)))
        mode = (mode == MODE_GLO) ? MODE_MATRIX : MODE_GLO;
    last_btn = btn;
}

void app_input_only(void) { input(); }

void app_ui_init(void)
{
    lcd_fill(0, 0, 240, 240, C_BLACK);
}

void app_frame(void)
{
    input();
    if (mode == MODE_MATRIX)
        draw_matrix();
    else if (mode == MODE_GLO)
        cv_text(10, 100, &FONT_B, "GLO", C_WHITE);
}

void app_say(const char *a, const char *b) { (void)a; (void)b; }
