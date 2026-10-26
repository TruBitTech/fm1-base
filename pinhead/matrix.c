/* SPDX-License-Identifier: MPL-2.0 */
#include "matrix.h"

void matrix_init(MatrixState *m)
{
    matrix_clear(m);
    m->full_page = false;
    m->cursor_row = 0;
    m->cursor_col = 0;
}

void matrix_clear(MatrixState *m)
{
    for (int r = 0; r < MATRIX_ROWS; r++)
        for (int c = 0; c < MATRIX_COLS; c++)
            m->pins[r][c] = PIN_NONE;
}

void matrix_set_view(MatrixState *m, bool full_page)
{
    m->full_page = full_page;
}

void matrix_move_cursor(MatrixState *m, int d_row, int d_col)
{
    int size = m->full_page ? MATRIX_ROWS : CORE_SIZE;
    if (m->full_page) {
        /* hard stop */
        int nr = (int)m->cursor_row + d_row;
        int nc = (int)m->cursor_col + d_col;
        if (nr < 0) nr = 0;
        if (nr >= size) nr = size - 1;
        if (nc < 0) nc = 0;
        if (nc >= size) nc = size - 1;
        m->cursor_row = (uint8_t)nr;
        m->cursor_col = (uint8_t)nc;
    } else {
        /* wrap */
        m->cursor_row = (uint8_t)((m->cursor_row + d_row + size) % size);
        m->cursor_col = (uint8_t)((m->cursor_col + d_col + size) % size);
    }
}

void matrix_cycle_pin(MatrixState *m)
{
    PinType p = m->pins[m->cursor_row][m->cursor_col];
    m->pins[m->cursor_row][m->cursor_col] = (PinType)((p + 1) & 3);
}

PinType matrix_get(const MatrixState *m, uint8_t row, uint8_t col)
{
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return PIN_NONE;
    return m->pins[row][col];
}
