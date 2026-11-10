/* SPDX-License-Identifier: MPL-2.0
 * Pin Head application — Synthi-style matrix on fm1-base
 */
#include "app.h"
#include "fastmath.h"
#include "matrix.c"
#include "joystick.c"
#include "engine.c"
#include "presets.c"

enum { MODE_MATRIX, MODE_PLAY, MODE_GLO };
static uint8_t mode = MODE_MATRIX;
static MatrixState matrix;
static JoystickState joystick;
static Engine engine;
static uint8_t glo_page = 0;
static uint8_t preset_idx = 0;
static int8_t octave = 0;
static uint32_t keys_held;

static uint8_t header_style = 0;
static uint8_t grid_style = 0;

static float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    float L[256], R[256];
    if (n > 256) n = 256;
    engine_process(&engine, L, R, (int)n);
    float g = (float)master_q12 / 4096.0f * 3500000.0f;
    for (uint32_t i = 0; i < n; i++) {
        lr[2 * i]     = (int32_t)(L[i] * g);
        lr[2 * i + 1] = (int32_t)(R[i] * g);
    }
}

void app_panic(void) { engine_set_gate(&engine, false); }

void app_init(void)
{
    matrix_init(&matrix);
    joystick_init(&joystick);
    engine_init(&engine, &matrix, &joystick);
    preset_idx = 0;
    preset_load(&matrix, 0);
    engine_set_note(&engine, 220.0f);
    engine_set_gate(&engine, false);
}

static const char *COL_HEAD[] = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P"};
static const char *ROW_HEAD[] = {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};

static const char *COL_LABEL[] = {
    "Out1", "Mtr", "Out2", "Env", "RngA", "RngB", "Rev", "Filt",
    "O1F", "O2F", "O3F", "Dec", "Sp", "FF", "Lv1", "Tape"
};

static const char *ROW_LABEL[] = {
    "O1s", "O1w", "O2p", "O2t", "O3p", "O3t", "Nse", "In1",
    "In2", "Flt", "Trp", "EnS", "Rng", "Rev", "StX", "Tape"
};

static void draw_vert(int x, int y, const char *s)
{
    int i = 0;
    while (s[i] && i < 6) {
        char ch[2] = {s[i], 0};
        cv_text(x, y - i * 7, &FONT_XS, ch, C_DIM);
        i++;
    }
}

static void draw_matrix(void)
{
    int size = matrix.full_page ? VIEW_ROWS : CORE_SIZE;
    int top = 18;
    int bottom = 16;
    int left = 14;
    int avail_h = 240 - top - bottom;
    int avail_w = 240 - left - 4;
    int cell = avail_h / size;
    if (cell > avail_w / size) cell = avail_w / size;
    if (cell < 6) cell = 6;
    int grid = cell * size;
    int ox = left + (avail_w - grid) / 2;
    int oy = top;

    int sr = matrix.full_page ? matrix.scroll_row : 0;
    int sc = matrix.full_page ? matrix.scroll_col : 0;

    cv_begin(240, 240, C_BLACK);

    for (int c = 0; c < size; c++) {
        int abs_c = sc + c;
        int px = ox + c * cell + cell / 2 - 2;
        if (header_style == 0)
            cv_text(px, 1, &FONT_XS, COL_HEAD[abs_c], C_GRAY);
        else
            draw_vert(px, oy - 6, COL_LABEL[abs_c]);
    }
    for (int r = 0; r < size; r++) {
        int abs_r = sr + r;
        int py = oy + r * cell + cell / 2 - 3;
        if (header_style == 0)
            cv_text(1, py, &FONT_XS, ROW_HEAD[abs_r], C_GRAY);
        else
            cv_text(1, py - 1, &FONT_XS, ROW_LABEL[abs_r], C_DIM);
    }

    if (grid_style == 1) {
        for (int i = 0; i <= size; i++) {
            int x = ox + i * cell;
            int y = oy + i * cell;
            cv_rect(x, oy, 1, grid, C_LINE);
            cv_rect(ox, y, grid, 1, C_LINE);
        }
    } else if (grid_style == 2) {
        for (int r = 0; r < size; r++)
            for (int c = 0; c < size; c++)
                if ((r + c) & 1)
                    cv_rect(ox + c * cell + 1, oy + r * cell + 1, cell - 2, cell - 2, C_LINE);
    }

    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            int abs_r = sr + r;
            int abs_c = sc + c;
            int px = ox + c * cell + cell / 2;
            int py = oy + r * cell + cell / 2;
            PinType pin = matrix.pins[abs_r][abs_c];
            uint16_t col = (grid_style == 0) ? C_DIM : C_BLACK;
            if (pin == PIN_WHITE) col = C_WHITE;
            else if (pin == PIN_RED) col = C_HI;
            else if (pin == PIN_GREEN) col = C_GRAY;
            cv_rect(px - 2, py - 2, 4, 4, col);
            if (abs_c == matrix.cursor_col && abs_r == matrix.cursor_row) {
                cv_rect(px - 5, py - 5, 10, 1, C_HI);
                cv_rect(px - 5, py + 4, 10, 1, C_HI);
                cv_rect(px - 5, py - 4, 1, 8, C_HI);
                cv_rect(px + 4, py - 4, 1, 8, C_HI);
            }
        }
    }

    cv_text(2, 226, &FONT_XS, matrix.full_page ? "FULL" : "CORE", C_GRAY);
    cv_text(40, 226, &FONT_XS, factory_presets[preset_idx].name, C_HI);
    cv_blit(0, 0);
}

static void draw_glo(void)
{
    static const char *PAGES[] = {"LUSH REVERB", "PING-PONG", "TAPE DELAY", "JOYSTICK", "DISPLAY"};
    cv_begin(240, 240, C_BLACK);
    cv_text(10, 20, &FONT_B, PAGES[glo_page], C_WHITE);
    if (glo_page == 0) {
        cv_text(10, 60, &FONT_XS, "K1 Mix", C_GRAY);
        cv_text(10, 80, &FONT_XS, "K2 Decay", C_GRAY);
    } else if (glo_page == 1) {
        cv_text(10, 60, &FONT_XS, "K1 Mix", C_GRAY);
        cv_text(10, 80, &FONT_XS, "K2 Time", C_GRAY);
        cv_text(10, 100, &FONT_XS, "K3 Fb", C_GRAY);
    } else if (glo_page == 2) {
        cv_text(10, 60, &FONT_XS, "K1 Time", C_GRAY);
        cv_text(10, 80, &FONT_XS, "K2 Fb", C_GRAY);
    } else if (glo_page == 3) {
        cv_text(10, 60, &FONT_XS, "K1 Range X", C_GRAY);
        cv_text(10, 80, &FONT_XS, "K2 Range Y", C_GRAY);
    } else {
        cv_text(10, 50, &FONT_XS, "K1 Headers", C_GRAY);
        cv_text(10, 70, &FONT_XS, header_style ? "Labeled" : "A / 1", C_HI);
        cv_text(10, 100, &FONT_XS, "K2 Grid", C_GRAY);
        const char *g = grid_style == 0 ? "Solid" : grid_style == 1 ? "Lines" : "Checker";
        cv_text(10, 120, &FONT_XS, g, C_HI);
        cv_text(10, 160, &FONT_XS, "Exit GLO to see matrix", C_DIM);
    }
    cv_blit(0, 0);
}

static void draw_play(void)
{
    cv_begin(240, 240, C_BLACK);
    cv_text(10, 20, &FONT_B, "PLAY", C_WHITE);
    cv_text(10, 50, &FONT_XS, "K1 = Joystick X", C_GRAY);
    cv_text(10, 70, &FONT_XS, "K2 = Joystick Y", C_GRAY);
    cv_text(10, 100, &FONT_XS, "Keys play notes", C_HI);
    cv_blit(0, 0);
}

static void note_on_key(int key)
{
    int note = 60 + key + octave * 12;
    float freq = 440.0f * fm_exp2f((float)(note - 69) / 12.0f);
    engine_set_note(&engine, freq);
    engine_set_gate(&engine, true);
}

static void input(void)
{
    uint32_t btn = plat_buttons();
    uint32_t keys = plat_keys();
    int32_t e;

    for (int k = 0; k < 27; k++) {
        uint32_t bit = 1u << k;
        if ((keys & bit) && !(keys_held & bit))
            note_on_key(k);
        else if (!(keys & bit) && (keys_held & bit))
            engine_set_gate(&engine, false);
    }
    keys_held = keys;

    if ((e = plat_enc(EN_SELECT))) {
        if (mode == MODE_MATRIX)
            matrix_set_view(&matrix, !matrix.full_page);
        else if (mode == MODE_GLO)
            glo_page = (uint8_t)((glo_page + (e > 0 ? 1 : 4)) % 5);
    }

    if (mode == MODE_MATRIX) {
        if ((e = plat_enc(EN_K1)))
            matrix_move_cursor(&matrix, 0, e);
        if ((e = plat_enc(EN_K2)))
            matrix_move_cursor(&matrix, e, 0);
        if ((e = plat_enc(EN_K3)))
            matrix_cycle_pin(&matrix);
        if ((e = plat_enc(EN_PRESET))) {
            preset_idx = (uint8_t)((preset_idx + e + PRESET_COUNT) % PRESET_COUNT);
            preset_load(&matrix, preset_idx);
        }
    } else if (mode == MODE_PLAY) {
        float rx = 0.0f, ry = 0.0f;
        if ((e = plat_enc(EN_K1))) rx = e * 0.05f;
        if ((e = plat_enc(EN_K2))) ry = e * 0.05f;
        joystick_set_from_knobs(&joystick, rx, ry);
    } else if (mode == MODE_GLO) {
        if (glo_page == 4) {
            if ((e = plat_enc(EN_K1)))
                header_style = (header_style + (e > 0 ? 1 : 1)) & 1;
            if ((e = plat_enc(EN_K2)))
                grid_style = (uint8_t)((grid_style + (e > 0 ? 1 : 2)) % 3);
        } else {
            if ((e = plat_enc(EN_K1))) {
                if (glo_page == 0) engine.reverb_mix = clampf(engine.reverb_mix + e * 0.05f, 0.0f, 1.0f);
                else if (glo_page == 1) engine.delay_mix = clampf(engine.delay_mix + e * 0.05f, 0.0f, 1.0f);
                else if (glo_page == 2) engine.tape.time = clampf(engine.tape.time + e * 0.02f, 0.05f, 1.0f);
                else joystick.range_x = clampf(joystick.range_x + e * 0.05f, 0.2f, 2.0f);
            }
            if ((e = plat_enc(EN_K2))) {
                if (glo_page == 0) engine.reverb_decay = clampf(engine.reverb_decay + e * 0.03f, 0.1f, 0.95f);
                else if (glo_page == 1) engine.delay_time = clampf(engine.delay_time + e * 0.02f, 0.05f, 1.0f);
                else if (glo_page == 2) engine.tape.feedback = clampf(engine.tape.feedback + e * 0.03f, 0.0f, 0.9f);
                else joystick.range_y = clampf(joystick.range_y + e * 0.05f, 0.2f, 2.0f);
            }
            if ((e = plat_enc(EN_K3)) && glo_page == 1)
                engine.delay_fb = clampf(engine.delay_fb + e * 0.03f, 0.0f, 0.9f);
        }
    }

    static uint32_t last_btn;
    if ((btn & (1u << B_GLO)) && !(last_btn & (1u << B_GLO)))
        mode = (mode == MODE_GLO) ? MODE_MATRIX : MODE_GLO;
    if ((btn & (1u << B_PLAY)) && !(last_btn & (1u << B_PLAY)))
        mode = (mode == MODE_PLAY) ? MODE_MATRIX : MODE_PLAY;
    if ((btn & (1u << B_ARP)) && !(last_btn & (1u << B_ARP))) {
        preset_idx = (preset_idx + 1) % PRESET_COUNT;
        preset_load(&matrix, preset_idx);
    }
    if ((btn & (1u << B_EDIT)) && !(last_btn & (1u << B_EDIT)) && mode == MODE_MATRIX)
        matrix_cycle_pin(&matrix);
    if ((btn & (1u << B_OCTUP)) && !(last_btn & (1u << B_OCTUP)))
        octave = (int8_t)(octave < 2 ? octave + 1 : 2);
    if ((btn & (1u << B_OCTDN)) && !(last_btn & (1u << B_OCTDN)))
        octave = (int8_t)(octave > -2 ? octave - 1 : -2);
    last_btn = btn;
}

void app_input_only(void) { input(); }

void app_ui_init(void) { lcd_fill(0, 0, 240, 240, C_BLACK); }

void app_frame(void)
{
    input();
    if (mode == MODE_MATRIX)
        draw_matrix();
    else if (mode == MODE_GLO)
        draw_glo();
    else if (mode == MODE_PLAY)
        draw_play();
}

void app_say(const char *a, const char *b) { (void)a; (void)b; }
