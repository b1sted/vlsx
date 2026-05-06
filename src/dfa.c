/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "dfa.h"

bool run_dfa(const dfa_t *dfa, const char *input) {
    if (!dfa || !input || !input[0])
        return false;

    int state = dfa->start_state;
    for (int i = 0; input[i]; i++) {
        int cls = dfa->get_class(input[i]);
        state = dfa->transitions[state * dfa->num_classes + cls];
        if (state == S_ERR)
            return false;
    }

    return dfa->is_final[state];
}
