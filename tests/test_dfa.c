/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "common/dfa.h"
#include "test_runner.h"

void test_dfa(void);

typedef enum {
    TEST_DFA_DIGIT,
    TEST_DFA_OTHER,
    TEST_DFA_CLASS_COUNT,
} test_dfa_class_t;

typedef enum {
    TEST_DFA_START,
    TEST_DFA_DIGITS,
    TEST_DFA_STATE_COUNT,
} test_dfa_state_t;

static int test_dfa_class(char ch);

static const int test_dfa_transitions[TEST_DFA_STATE_COUNT][TEST_DFA_CLASS_COUNT] = {
    [TEST_DFA_START] = {TEST_DFA_DIGITS, DFA_STATE_ERROR},
    [TEST_DFA_DIGITS] = {TEST_DFA_DIGITS, DFA_STATE_ERROR},
};

static const bool test_dfa_finals[TEST_DFA_STATE_COUNT] = {
    [TEST_DFA_DIGITS] = true,
};

static const dfa_t test_dfa_table = {
    .num_states = TEST_DFA_STATE_COUNT,
    .num_classes = TEST_DFA_CLASS_COUNT,
    .start_state = TEST_DFA_START,
    .transitions = (const int *)test_dfa_transitions,
    .is_final = test_dfa_finals,
    .get_class = test_dfa_class,
};

static int test_dfa_class(char ch)
{
    return (ch >= '0' && ch <= '9') ? TEST_DFA_DIGIT : TEST_DFA_OTHER;
}

void test_dfa(void)
{
    TEST_CHECK(dfa_run(&test_dfa_table, "12345"));
    TEST_CHECK(!dfa_run(&test_dfa_table, "12a"));
    TEST_CHECK(!dfa_run(&test_dfa_table, ""));
    TEST_CHECK(!dfa_run(NULL, "1"));
    TEST_CHECK(!dfa_run(&test_dfa_table, NULL));
}
