/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TESTS_TEST_RUNNER_H
#define VLSX_TESTS_TEST_RUNNER_H

#include <stdio.h>

extern int tests_failed;

#define TEST_CHECK(condition)                                                                      \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);                   \
            tests_failed++;                                                                        \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#endif /* VLSX_TESTS_TEST_RUNNER_H */
