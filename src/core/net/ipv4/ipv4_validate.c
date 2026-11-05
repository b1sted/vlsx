/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv4/ipv4_validate.h"

#include <stdbool.h>

#include "common/dfa.h"

typedef enum {
    IPV4_C_DIGIT,
    IPV4_C_DOT,
    IPV4_C_SLASH,
    IPV4_C_OTHER,
    IPV4_NUM_CLASSES,
} ipv4_class_char_t;

typedef enum {
    IPV4_S_START,
    IPV4_S_OCTDIGIT,
    IPV4_S_DOT,
    IPV4_S_SLASH,
    IPV4_S_MSKDIGIT,
    IPV4_NUM_STATES,
} ipv4_state_t;

static int ipv4_char_class(char ch);

static const int ipv4_transitions[IPV4_NUM_STATES][IPV4_NUM_CLASSES] = {
    /*                       DIGIT              DOT             SLASH */
    [IPV4_S_START] = {IPV4_S_OCTDIGIT, DFA_STATE_ERROR, DFA_STATE_ERROR, DFA_STATE_ERROR},
    [IPV4_S_OCTDIGIT] = {IPV4_S_OCTDIGIT, IPV4_S_DOT, IPV4_S_SLASH, DFA_STATE_ERROR},
    [IPV4_S_DOT] = {IPV4_S_OCTDIGIT, DFA_STATE_ERROR, DFA_STATE_ERROR, DFA_STATE_ERROR},
    [IPV4_S_SLASH] = {IPV4_S_MSKDIGIT, DFA_STATE_ERROR, DFA_STATE_ERROR, DFA_STATE_ERROR},
    [IPV4_S_MSKDIGIT] = {IPV4_S_MSKDIGIT, DFA_STATE_ERROR, DFA_STATE_ERROR, DFA_STATE_ERROR},
};

static const bool ipv4_finals[IPV4_NUM_STATES] = {
    [IPV4_S_MSKDIGIT] = true,
};

static const dfa_t ipv4_validator = {
    .num_states = IPV4_NUM_STATES,
    .num_classes = IPV4_NUM_CLASSES,
    .start_state = IPV4_S_START,
    .transitions = (const int *)ipv4_transitions,
    .is_final = ipv4_finals,
    .get_class = ipv4_char_class,
};

static int ipv4_char_class(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return IPV4_C_DIGIT;
    }

    if (ch == '.') {
        return IPV4_C_DOT;
    }

    if (ch == '/') {
        return IPV4_C_SLASH;
    }

    return IPV4_C_OTHER;
}

bool ipv4_validate(const char *input)
{
    return dfa_run(&ipv4_validator, input);
}

bool ipv4_accept(char ch)
{
    return ipv4_char_class(ch) != IPV4_C_OTHER;
}
