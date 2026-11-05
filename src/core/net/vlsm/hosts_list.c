/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/vlsm/hosts_list.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common/dfa.h"
#include "common/result.h"

typedef enum {
    HOSTS_C_DIGIT,
    HOSTS_C_COMMA,
    HOSTS_C_SPACE,
    HOSTS_C_OTHER,
    HOSTS_NUM_CLASSES,
} hosts_class_t;

typedef enum {
    HOSTS_S_START,
    HOSTS_S_DIGIT,
    HOSTS_S_COMMA,
    HOSTS_S_SPACE,
    HOSTS_NUM_STATES,
} hosts_state_t;

static int hosts_char_class(char ch);
static int compare_desc(const void *a, const void *b);

static const int hosts_transitions[HOSTS_NUM_STATES][HOSTS_NUM_CLASSES] = {
    /*                    DIGIT            COMMA            SPACE */
    [HOSTS_S_START] = {HOSTS_S_DIGIT, DFA_STATE_ERROR, DFA_STATE_ERROR, DFA_STATE_ERROR},
    [HOSTS_S_DIGIT] = {HOSTS_S_DIGIT, HOSTS_S_COMMA, DFA_STATE_ERROR, DFA_STATE_ERROR},
    [HOSTS_S_COMMA] = {HOSTS_S_DIGIT, DFA_STATE_ERROR, HOSTS_S_SPACE, DFA_STATE_ERROR},
    [HOSTS_S_SPACE] = {HOSTS_S_DIGIT, DFA_STATE_ERROR, DFA_STATE_ERROR, DFA_STATE_ERROR},
};

static const bool hosts_finals[HOSTS_NUM_STATES] = {
    [HOSTS_S_DIGIT] = true,
};

static const dfa_t hosts_validator = {
    .num_states = HOSTS_NUM_STATES,
    .num_classes = HOSTS_NUM_CLASSES,
    .start_state = HOSTS_S_START,
    .transitions = (const int *)hosts_transitions,
    .is_final = hosts_finals,
    .get_class = hosts_char_class,
};

static int hosts_char_class(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return HOSTS_C_DIGIT;
    }

    if (ch == ',') {
        return HOSTS_C_COMMA;
    }

    if (ch == ' ') {
        return HOSTS_C_SPACE;
    }

    return HOSTS_C_OTHER;
}

static int compare_desc(const void *a, const void *b)
{
    const uint32_t first = *(const uint32_t *)a;
    const uint32_t second = *(const uint32_t *)b;

    if (first > second) {
        return -1;
    }

    if (first < second) {
        return 1;
    }

    return 0;
}

bool hosts_validate(const char *input)
{
    return dfa_run(&hosts_validator, input);
}

bool hosts_accept(char ch)
{
    return hosts_char_class(ch) != HOSTS_C_OTHER;
}

void hosts_sort_descending(uint32_t *hosts, size_t count)
{
    if (hosts == NULL || count < 2) {
        return;
    }

    qsort(hosts, count, sizeof(*hosts), compare_desc);
}

bool hosts_parse(const char *input, uint32_t *hosts, size_t capacity, size_t *count,
                 calc_status_t *status)
{
    char *copy = NULL;
    bool result = false;

    if (hosts == NULL || count == NULL || status == NULL) {
        return false;
    }

    *count = 0;
    calc_status_clear(status);

    if (!hosts_validate(input)) {
        calc_status_fail(status, CALC_ERR_SYNTAX,
                         "Invalid input.\nPlease correct it and try again.");
        return false;
    }

    copy = strdup(input);
    if (copy == NULL) {
        calc_status_fail(status, CALC_ERR_ALLOCATION, "Unable to allocate host list buffer.");
        return false;
    }

    char *saveptr = NULL;
    char *token = strtok_r(copy, " ,", &saveptr);

    while (token != NULL && *count < capacity) {
        char *endptr = NULL;
        const uint64_t value = strtoull(token, &endptr, 10);

        if (endptr == token || (endptr != NULL && *endptr != '\0')) {
            calc_status_fail(status, CALC_ERR_SYNTAX,
                             "Invalid input.\nPlease correct it and try again.");
            goto cleanup;
        }

        if (value == 0) {
            token = strtok_r(NULL, " ,", &saveptr);
            continue;
        }

        if (value > UINT32_C(4294967294)) {
            calc_status_fail(status, CALC_ERR_RANGE,
                             "Invalid input.\nHost value too large for IPv4.");
            goto cleanup;
        }

        hosts[*count] = (uint32_t)value;
        (*count)++;

        token = strtok_r(NULL, " ,", &saveptr);
    }

    if (token != NULL) {
        calc_status_fail(status, CALC_ERR_CAPACITY, "Invalid input.\nToo many host list entries.");
        goto cleanup;
    }

    hosts_sort_descending(hosts, *count);

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';
    result = true;

cleanup:
    free(copy);
    return result;
}
