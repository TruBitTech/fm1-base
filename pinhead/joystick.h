/* SPDX-License-Identifier: MPL-2.0 */
#ifndef PINHEAD_JOYSTICK_H
#define PINHEAD_JOYSTICK_H

#include <stdint.h>

typedef struct {
    float x, y;
    float range_x, range_y;
} JoystickState;

void joystick_init(JoystickState *j);
void joystick_set_from_knobs(JoystickState *j, float raw_x, float raw_y);
float joystick_mod_x(const JoystickState *j);
float joystick_mod_y(const JoystickState *j);

#endif
