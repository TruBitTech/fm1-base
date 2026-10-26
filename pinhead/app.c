/* SPDX-License-Identifier: MPL-2.0
 * Pin Head application — matrix-centric UI on fm1-base
 */
#include "app.h"
#include "matrix.h"
#include "joystick.h"

/* ------------------------------------------------------------------ state --- */
enum { MODE_MATRIX, MODE_PLAY, MODE_GLO };
static uint8_t mode = MODE_MATRIX;
static MatrixState matrix;
static JoystickState joystick;
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
    matrix_init(&matrix);
    joystick_init(&joystick);
}

/* ------------------------------------------------------------------ UI --- */
static const char *COL_HEAD[] = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P"};
static const char *ROW_HEAD[] = {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};

static void draw_matrix(void)
{
    int size = matrix.full_page ? 16 : 10;
    int cell = matrix.full_page ? 12 : 18;
    int ox = 28, oy = 22;

    cv_begin(240, 210, C_BLACK);

    /* sticky column headers */
    for (int c = 0; c < size; c++) {
        int px = ox + c * cell;
        cv_text(px - 3, 4, &FONT_XS, COL_HEAD[c], C_GRAY);
    }
    /* sticky row headers */
    for (int r = 0; r < size; r++) {
        int py = oy + r * cell;
        cv_text(4, py - 4, &FONT_XS, ROW_HEAD[r], C_GRAY);
    }

    /* pins */
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            int px = ox + c * cell;
            int py = oy + r * cell;
            PinType p = matrix.pins[r][c];
            uint16_t col = C_DIM;
            if (p == PIN_WHITE) col = C_WHITE;
            else if (p == PIN_RED) col = C_RED;
            else if (p == PIN_GREEN) col = C_GREEN;
            cv_rect(px - 2, py - 2, 4, 4, col);
            if (c == matrix.cursor_col && r == matrix.cursor_row)
                cv_rect(px - 5, py - 5, 10, 10, C_HI);
        }
    }

    cv_text(4, 200, &FONT_XS, matrix.full_page ? "FULL" : "CORE", C_GRAY);
    cv_blit(0, 0);
}

static void draw_glo(void)
{
    static const char *PAGES[] = {"LUSH REVERB", "PING-PONG", "TAPE DELAY", "JOYSTICK"};
    cv_begin(240, 200, C_BLACK);
    cv_text(10, 20, &FONT_B, PAGES[glo_page], C_WHITE);
    cv_text(10, 50, &FONT_XS, "K1 / K2 adjust", C_GRAY);
    cv_blit(0, 0);
}

static void input(void)
{
    uint32_t btn = plat_buttons();
    int32_t e;

    if ((e = plat_enc(EN_SELECT))) {
        if (mode == MODE_MATRIX)
            matrix_set_view(&matrix, !matrix.full_page);
        else if (mode == MODE_GLO)
            glo_page = (uint8_t)((glo_page + e) & 3);
    }

    if (mode == MODE_MATRIX) {
        if ((e = plat_enc(EN_K1)))
            matrix_move_cursor(&matrix, 0, e);
        if ((e = plat_enc(EN_K2)))
            matrix_move_cursor(&matrix, e, 0);
        if ((e = plat_enc(EN_K3)) && e)
            matrix_cycle_pin(&matrix);
    } else if (mode == MODE_PLAY) {
        float rx = 0.0f, ry = 0.0f;
        if ((e = plat_enc(EN_K1))) rx = e * 0.05f;
        if ((e = plat_enc(EN_K2))) ry = e * 0.05f;
        joystick_set_from_knobs(&joystick, rx, ry);
    }

    static uint32_t last_btn;
    if ((btn & (1u << B_GLO)) && !(last_btn & (1u << B_GLO)))
        mode = (mode == MODE_GLO) ? MODE_MATRIX : MODE_GLO;
    if ((btn & (1u << B_PLAY)) && !(last_btn & (1u << B_PLAY)))
        mode = (mode == MODE_PLAY) ? MODE_MATRIX : MODE_PLAY;
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
        draw_glo();
    else if (mode == MODE_PLAY)
        cv_text(10, 100, &FONT_B, "PLAY", C_WHITE);
}

void app_say(const char *a, const char *b) { (void)a; (void)b; }
