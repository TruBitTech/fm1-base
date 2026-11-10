/* SPDX-License-Identifier: MPL-2.0 */
#ifndef PINHEAD_MATRIX_H
#define PINHEAD_MATRIX_H

#include <stdint.h>
#include <stdbool.h>

#define MATRIX_ROWS     16
#define MATRIX_COLS     16
#define CORE_SIZE       10
#define VIEW_ROWS       10
#define VIEW_COLS       10

typedef enum {
    PIN_NONE  = 0,
    PIN_WHITE = 1,
    PIN_RED   = 2,
    PIN_GREEN = 3
} PinType;

static inline float pin_gain(PinType t)
{
    switch (t) {
        case PIN_WHITE:
        case PIN_RED:   return 1.0f;
        case PIN_GREEN: return 0.25f;
        default:        return 0.0f;
    }
}

typedef struct {
    PinType pins[MATRIX_ROWS][MATRIX_COLS];
    bool    full_page;
    uint8_t cursor_row;
    uint8_t cursor_col;
    uint8_t scroll_row;
    uint8_t scroll_col;
} MatrixState;

void matrix_init(MatrixState *m);
void matrix_clear(MatrixState *m);
void matrix_set_view(MatrixState *m, bool full_page);
void matrix_move_cursor(MatrixState *m, int d_row, int d_col);
void matrix_cycle_pin(MatrixState *m);
PinType matrix_get(const MatrixState *m, uint8_t row, uint8_t col);

#endif
