/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/result.h"
#include "core/net/vlsm/vlsm_alloc.h"
#include "core/net/vlsm/vlsm_types.h"
#include "test_runner.h"

void test_vlsm(void);

static void test_vlsm_basic(void);
static void test_vlsm_sorting(void);
static void test_vlsm_errors(void);

static void test_vlsm_basic(void)
{
    vlsm_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(vlsm_plan("192.168.0.0/24", "100, 50, 20", &result, &status));
    TEST_CHECK(result.entry_count == 3);
    TEST_CHECK(result.base_prefix == 24);
    TEST_CHECK(result.capacity == 254);
    TEST_CHECK(result.base_network.octets[0] == 192);
    TEST_CHECK(result.base_network.octets[1] == 168);
    TEST_CHECK(result.base_network.octets[2] == 0);
    TEST_CHECK(result.base_network.octets[3] == 0);

    TEST_CHECK(result.entries[0].requested_hosts == 100);
    TEST_CHECK(result.entries[0].prefix == 25);
    TEST_CHECK(result.entries[0].network.octets[3] == 0);
    TEST_CHECK(result.entries[0].broadcast.octets[3] == 127);
    TEST_CHECK(result.entries[0].allocated_hosts == 126);
    TEST_CHECK(result.entries[0].unused_hosts == 26);

    TEST_CHECK(result.entries[1].requested_hosts == 50);
    TEST_CHECK(result.entries[1].prefix == 26);
    TEST_CHECK(result.entries[1].network.octets[3] == 128);
    TEST_CHECK(result.entries[1].broadcast.octets[3] == 191);

    TEST_CHECK(result.entries[2].requested_hosts == 20);
    TEST_CHECK(result.entries[2].prefix == 27);
    TEST_CHECK(result.entries[2].network.octets[3] == 192);
    TEST_CHECK(result.entries[2].broadcast.octets[3] == 223);

    TEST_CHECK(result.allocated_hosts == 218);
    TEST_CHECK(result.unallocated_hosts == 36);

    TEST_CHECK(vlsm_plan("10.0.0.0/24", "5", &result, &status));
    TEST_CHECK(result.entry_count == 1);
    TEST_CHECK(result.entries[0].prefix == 29);
    TEST_CHECK(result.entries[0].allocated_hosts == 6);
    TEST_CHECK(result.entries[0].unused_hosts == 1);

    TEST_CHECK(vlsm_plan("10.0.0.0/24", "0, 5", &result, &status));
    TEST_CHECK(result.entry_count == 1);
    TEST_CHECK(result.entries[0].requested_hosts == 5);
}

static void test_vlsm_sorting(void)
{
    vlsm_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(vlsm_plan("10.0.0.0/24", "20, 100, 50", &result, &status));
    TEST_CHECK(result.entry_count == 3);
    TEST_CHECK(result.entries[0].requested_hosts == 100);
    TEST_CHECK(result.entries[1].requested_hosts == 50);
    TEST_CHECK(result.entries[2].requested_hosts == 20);
}

static void test_vlsm_errors(void)
{
    vlsm_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(!vlsm_plan("0.0.0.0/0", "1", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!vlsm_plan("10.0.0.0/24", "300", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_CAPACITY);

    TEST_CHECK(!vlsm_plan("10.0.0.0/24", "4294967295", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!vlsm_plan("10.0.0.0/24",
                          "1, 2, 3, 4, 5, 6, 7, 8, 9, 10, "
                          "11, 12, 13, 14, 15, 16, 17",
                          &result, &status));
    TEST_CHECK(status.code == CALC_ERR_CAPACITY);

    TEST_CHECK(!vlsm_plan("192.168.0.0/24", "128, 128", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_CAPACITY);
}

void test_vlsm(void)
{
    test_vlsm_basic();
    test_vlsm_sorting();
    test_vlsm_errors();
}
