/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/result.h"
#include "core/net/vlsm/hosts_list.h"
#include "test_runner.h"

void test_hosts_list(void);

static void test_hosts_parse_valid(void);
static void test_hosts_parse_invalid(void);
static void test_hosts_validation(void);

static void test_hosts_parse_valid(void)
{
    uint32_t values[16] = {0};
    size_t count = 0;
    calc_status_t status = {0};

    TEST_CHECK(hosts_parse("500, 100, 2", values, 16, &count, &status));
    TEST_CHECK(count == 3);
    TEST_CHECK(values[0] == 500);
    TEST_CHECK(values[1] == 100);
    TEST_CHECK(values[2] == 2);

    TEST_CHECK(hosts_parse("0, 0, 5", values, 16, &count, &status));
    TEST_CHECK(count == 1);
    TEST_CHECK(values[0] == 5);
}

static void test_hosts_parse_invalid(void)
{
    uint32_t values[16] = {0};
    size_t count = 0;
    calc_status_t status = {0};

    TEST_CHECK(!hosts_parse("1,,2", values, 16, &count, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!hosts_parse("", values, 16, &count, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!hosts_parse("4294967295", values, 16, &count, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(
        !hosts_parse("1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17", values, 16, &count, &status));
    TEST_CHECK(status.code == CALC_ERR_CAPACITY);
}

static void test_hosts_validation(void)
{
    TEST_CHECK(hosts_validate("1, 2"));
    TEST_CHECK(hosts_validate("500,100,2"));
    TEST_CHECK(!hosts_validate("1,"));
    TEST_CHECK(!hosts_validate("1, "));
    TEST_CHECK(!hosts_validate("1 2"));
    TEST_CHECK(hosts_accept('5'));
    TEST_CHECK(hosts_accept(','));
    TEST_CHECK(hosts_accept(' '));
    TEST_CHECK(!hosts_accept('x'));
}

void test_hosts_list(void)
{
    test_hosts_parse_valid();
    test_hosts_parse_invalid();
    test_hosts_validation();
}
