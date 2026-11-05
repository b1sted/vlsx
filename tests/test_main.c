/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdio.h>

#include "test_runner.h"

void test_ipv4(void);
void test_vlsm(void);
void test_hosts_list(void);
void test_dfa(void);

int tests_failed = 0;

int main(void)
{
    test_ipv4();
    test_vlsm();
    test_hosts_list();
    test_dfa();

    if (tests_failed != 0) {
        fprintf(stderr, "%d test check(s) failed\n", tests_failed);
        return 1;
    }

    printf("All core tests passed.\n");
    return 0;
}
