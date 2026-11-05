/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_COMMON_DFA_H
#define VLSX_COMMON_DFA_H

#include <stdbool.h>

#define DFA_STATE_ERROR (-1)

typedef struct {
    int num_states;
    int num_classes;

    int start_state;
    const int *transitions;
    const bool *is_final;
    int (*get_class)(char c);
} dfa_t;

bool dfa_run(const dfa_t *dfa, const char *input);

#endif /* VLSX_COMMON_DFA_H */
