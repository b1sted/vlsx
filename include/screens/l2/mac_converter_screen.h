/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_L2_MAC_CONVERTER_SCREEN_H
#define VLSX_SCREENS_L2_MAC_CONVERTER_SCREEN_H

#include <stdbool.h>

#include "common/input_field.h"
#include "common/limits.h"
#include "common/result.h"
#include "common/ui_event.h"
#include "core/net/mac/mac_types.h"

typedef struct {
    input_field_t input;
    char buf[MAC_INPUT_MAX];
    bool has_result;
    bool is_valid;
    calc_status_t status;
    mac_result_t result;
} mac_converter_screen_t;

void mac_converter_screen_init(mac_converter_screen_t *screen);
void mac_converter_screen_confirm(mac_converter_screen_t *screen);
bool mac_converter_screen_handle(mac_converter_screen_t *screen, const ui_event_t *event);
const input_field_t *mac_converter_screen_active_input(const mac_converter_screen_t *screen);

#endif /* VLSX_SCREENS_L2_MAC_CONVERTER_SCREEN_H */
