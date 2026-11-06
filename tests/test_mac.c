/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common/result.h"
#include "core/net/mac/mac_calc.h"
#include "core/net/mac/mac_multicast.h"
#include "core/net/mac/mac_parse.h"
#include "core/net/mac/mac_types.h"
#include "test_runner.h"

void test_mac(void);

static void test_mac_target_example(void);
static void test_mac_notations(void);
static void test_mac_bit_analysis(void);
static void test_mac_errors(void);
static void test_mac_multicast_target_example(void);
static void test_mac_multicast_boundaries(void);
static void test_mac_multicast_errors(void);

static void test_mac_target_example(void)
{
    mac_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(mac_calculate("F4:46:37:55:24:86", &result, &status));
    TEST_CHECK(result.address.octets[0] == UINT8_C(0xF4));
    TEST_CHECK(result.address.octets[1] == UINT8_C(0x46));
    TEST_CHECK(result.address.octets[2] == UINT8_C(0x37));
    TEST_CHECK(result.address.octets[3] == UINT8_C(0x55));
    TEST_CHECK(result.address.octets[4] == UINT8_C(0x24));
    TEST_CHECK(result.address.octets[5] == UINT8_C(0x86));
    TEST_CHECK(strcmp(result.colon, "F4:46:37:55:24:86") == 0);
    TEST_CHECK(strcmp(result.oui, "F4:46:37") == 0);
    TEST_CHECK(strcmp(result.nic, "55:24:86") == 0);
    TEST_CHECK(strcmp(result.cisco, "F446.3755.2486") == 0);
    TEST_CHECK(strcmp(result.binary, "11110100.01000110.00110111.01010101.00100100.10000110") == 0);
    TEST_CHECK(strcmp(result.first_octet_binary, "11110100") == 0);
    TEST_CHECK(result.cast == MAC_CAST_UNICAST);
    TEST_CHECK(result.admin == MAC_ADMIN_UAA);
    TEST_CHECK(strcmp(mac_cast_name(result.cast), "Unicast") == 0);
    TEST_CHECK(strcmp(mac_admin_name(result.admin), "UAA") == 0);
}

static void test_mac_notations(void)
{
    mac_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(mac_validate("F4:46:37:55:24:86"));
    TEST_CHECK(mac_validate("f4-46-37-55-24-86"));
    TEST_CHECK(mac_validate("f446.3755.2486"));
    TEST_CHECK(mac_validate("f44637552486"));
    TEST_CHECK(mac_validate("F4:46:37:55:24:86"));
    TEST_CHECK(!mac_validate("F4-46-37:55-24-86"));
    TEST_CHECK(!mac_validate("F4:46:37-55:24:86"));
    TEST_CHECK(!mac_validate("F4.46.37.55.24.86"));

    TEST_CHECK(mac_calculate("f4-46-37-55-24-86", &result, &status));
    TEST_CHECK(strcmp(result.colon, "F4:46:37:55:24:86") == 0);

    TEST_CHECK(mac_calculate("f446.3755.2486", &result, &status));
    TEST_CHECK(strcmp(result.colon, "F4:46:37:55:24:86") == 0);

    TEST_CHECK(mac_calculate("f44637552486", &result, &status));
    TEST_CHECK(strcmp(result.colon, "F4:46:37:55:24:86") == 0);
    TEST_CHECK(strcmp(result.cisco, "F446.3755.2486") == 0);
}

static void test_mac_bit_analysis(void)
{
    mac_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(mac_calculate("F5:46:37:55:24:86", &result, &status));
    TEST_CHECK(result.cast == MAC_CAST_MULTICAST);
    TEST_CHECK(result.admin == MAC_ADMIN_UAA);

    TEST_CHECK(mac_calculate("F6:46:37:55:24:86", &result, &status));
    TEST_CHECK(result.cast == MAC_CAST_UNICAST);
    TEST_CHECK(result.admin == MAC_ADMIN_LAA);

    TEST_CHECK(mac_calculate("F7:46:37:55:24:86", &result, &status));
    TEST_CHECK(result.cast == MAC_CAST_MULTICAST);
    TEST_CHECK(result.admin == MAC_ADMIN_LAA);

    TEST_CHECK(mac_calculate("FF:FF:FF:FF:FF:FF", &result, &status));
    TEST_CHECK(result.cast == MAC_CAST_BROADCAST);
    TEST_CHECK(result.admin == MAC_ADMIN_LAA);
    TEST_CHECK(strcmp(result.first_octet_binary, "11111111") == 0);

    TEST_CHECK(mac_calculate("00:00:00:00:00:00", &result, &status));
    TEST_CHECK(result.cast == MAC_CAST_UNICAST);
    TEST_CHECK(result.admin == MAC_ADMIN_UAA);
}

static void test_mac_errors(void)
{
    mac_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(!mac_validate(NULL));
    TEST_CHECK(!mac_validate(""));
    TEST_CHECK(!mac_validate("F4:46:37:55:24"));
    TEST_CHECK(!mac_validate("F4:46:37:55:24:86:00"));
    TEST_CHECK(!mac_validate("F4:46:37:55:24:8"));
    TEST_CHECK(!mac_validate("F4:46:37:55:24:8G"));
    TEST_CHECK(!mac_validate("F446.3755.248"));
    TEST_CHECK(!mac_validate("F446.375524.86"));
    TEST_CHECK(!mac_validate("F446.3755.2486.00"));
    TEST_CHECK(!mac_validate("F4463755248"));
    TEST_CHECK(!mac_validate("F446375524860"));
    TEST_CHECK(!mac_validate("F4_46_37_55_24_86"));
    TEST_CHECK(!mac_validate(" F4:46:37:55:24:86"));
    TEST_CHECK(!mac_validate("F4:46:37:55:24:86 "));

    TEST_CHECK(!mac_calculate("not-a-mac", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
    TEST_CHECK(!status.ok);
    TEST_CHECK(!mac_calculate(NULL, &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
}

static void test_mac_multicast_target_example(void)
{
    mac_multicast_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(mac_multicast_generate("239.1.1.15", &result, &status));
    TEST_CHECK(result.ip == UINT32_C(0xEF01010F));
    TEST_CHECK(result.address.octets[0] == UINT8_C(0x01));
    TEST_CHECK(result.address.octets[1] == UINT8_C(0x00));
    TEST_CHECK(result.address.octets[2] == UINT8_C(0x5E));
    TEST_CHECK(result.address.octets[3] == UINT8_C(0x01));
    TEST_CHECK(result.address.octets[4] == UINT8_C(0x01));
    TEST_CHECK(result.address.octets[5] == UINT8_C(0x0F));
    TEST_CHECK(strcmp(result.ip_text, "239.1.1.15") == 0);
    TEST_CHECK(strcmp(result.oui, "01:00:5e") == 0);
    TEST_CHECK(strcmp(result.mac_text, "01:00:5e:01:01:0f") == 0);
    TEST_CHECK(strcmp(result.ip_binary, "11101111.00000001.00000001.00001111") == 0);
    TEST_CHECK(strcmp(result.extracted, "..........0000001.00000001.00001111") == 0);
    TEST_CHECK(status.ok);
    TEST_CHECK(status.code == CALC_OK);
}

static void test_mac_multicast_boundaries(void)
{
    mac_multicast_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(mac_multicast_generate("224.0.0.0", &result, &status));
    TEST_CHECK(strcmp(result.mac_text, "01:00:5e:00:00:00") == 0);

    TEST_CHECK(mac_multicast_generate("224.0.0.1", &result, &status));
    TEST_CHECK(strcmp(result.mac_text, "01:00:5e:00:00:01") == 0);

    TEST_CHECK(mac_multicast_generate("239.255.255.255", &result, &status));
    TEST_CHECK(strcmp(result.mac_text, "01:00:5e:7f:ff:ff") == 0);

    TEST_CHECK(mac_multicast_generate("224.1.1.1", &result, &status));
    TEST_CHECK(strcmp(result.mac_text, "01:00:5e:01:01:01") == 0);

    TEST_CHECK(mac_multicast_generate("239.1.1.1", &result, &status));
    TEST_CHECK(strcmp(result.mac_text, "01:00:5e:01:01:01") == 0);

    TEST_CHECK(mac_multicast_generate("224.128.1.1", &result, &status));
    TEST_CHECK(strcmp(result.mac_text, "01:00:5e:00:01:01") == 0);

    TEST_CHECK(mac_multicast_generate("239.128.1.1", &result, &status));
    TEST_CHECK(strcmp(result.mac_text, "01:00:5e:00:01:01") == 0);

    TEST_CHECK(mac_multicast_validate("239.1.1.15"));
    TEST_CHECK(!mac_multicast_validate("192.168.0.1"));
    TEST_CHECK(mac_multicast_accept('0'));
    TEST_CHECK(mac_multicast_accept('9'));
    TEST_CHECK(mac_multicast_accept('.'));
    TEST_CHECK(!mac_multicast_accept('/'));
    TEST_CHECK(!mac_multicast_accept('a'));
}

static void test_mac_multicast_errors(void)
{
    mac_multicast_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(!mac_multicast_validate(NULL));
    TEST_CHECK(!mac_multicast_validate(""));
    TEST_CHECK(!mac_multicast_validate("239.1.1"));
    TEST_CHECK(!mac_multicast_validate("239.1.1.15/24"));
    TEST_CHECK(!mac_multicast_validate("239.1.1.256"));
    TEST_CHECK(!mac_multicast_validate(" 239.1.1.15"));
    TEST_CHECK(!mac_multicast_validate("239.1.1.15 "));

    TEST_CHECK(!mac_multicast_generate("not-an-ip", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
    TEST_CHECK(!status.ok);

    TEST_CHECK(!mac_multicast_generate("192.168.0.1", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);
    TEST_CHECK(strcmp(status.message, "Not a Multicast IP") == 0);

    TEST_CHECK(!mac_multicast_generate("223.255.255.255", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!mac_multicast_generate("240.0.0.1", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!mac_multicast_generate(NULL, &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
}

void test_mac(void)
{
    test_mac_target_example();
    test_mac_notations();
    test_mac_bit_analysis();
    test_mac_errors();
    test_mac_multicast_target_example();
    test_mac_multicast_boundaries();
    test_mac_multicast_errors();
}
