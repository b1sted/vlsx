/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef DFA_H
#define DFA_H

#include <stdbool.h>

#define S_ERR -1

typedef struct {
    int num_states;
    int num_classes;

    int start_state;
    const int *transitions;
    const bool *is_final;
    int (*get_class)(char c);
} dfa_t;

bool run_dfa(const dfa_t *dfa, const char *input);

#endif /* DFA_H */
