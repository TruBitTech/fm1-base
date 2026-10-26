/* SPDX-License-Identifier: MPL-2.0
 * Pin Head — Synthi AKS-inspired firmware for M-VAVE FM-1
 */
#pragma once
#include <stdint.h>
#include "plat.h"

#define APP_NAME "PINHEAD"
#define APP_SPLASH_RGB RGB(20, 20, 20)
#define APP_BLOCK 256u

void app_init(void);
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12);
void app_panic(void);
void app_ui_init(void);
void app_frame(void);
void app_input_only(void);
void app_say(const char *a, const char *b);
