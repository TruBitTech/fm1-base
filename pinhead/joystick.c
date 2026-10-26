/* SPDX-License-Identifier: MPL-2.0 */
#include "joystick.h"

void joystick_init(JoystickState *j)
{
    j->x = j->y = 0.0f;
    j->range_x = j->range_y = 1.0f;
}

void joystick_set_from_knobs(JoystickState *j, float raw_x, float raw_y)
{
    /* raw -1..1 → limited ±1.5 V style */
    j->x = raw_x * j->range_x;
    j->y = raw_y * j->range_y;
    if (j->x > 1.5f) j->x = 1.5f;
    if (j->x < -1.5f) j->x = -1.5f;
    if (j->y > 1.5f) j->y = 1.5f;
    if (j->y < -1.5f) j->y = -1.5f;
}

float joystick_mod_x(const JoystickState *j) { return j->x; }
float joystick_mod_y(const JoystickState *j) { return j->y; }
