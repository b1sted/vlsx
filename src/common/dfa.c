/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "common/dfa.h"

#include <stdbool.h>
#include <stddef.h>

bool dfa_run(const dfa_t *dfa, const char *input)
{
    if (dfa == NULL || input == NULL || input[0] == '\0') {
        return false;
    }

    if (dfa->transitions == NULL || dfa->is_final == NULL || dfa->get_class == NULL) {
        return false;
    }

    int state = dfa->start_state;

    for (size_t i = 0; input[i] != '\0'; i++) {
        const int char_class = dfa->get_class(input[i]);
        state = dfa->transitions[(size_t)state * (size_t)dfa->num_classes + (size_t)char_class];

        if (state == DFA_STATE_ERROR) {
            return false;
        }
    }

    return dfa->is_final[state];
}
