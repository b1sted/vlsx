/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "input.h"
#include "termbox2.h"

#include "app.h"
#include "ui.h"

#include "screens/ip/v4.h"
#include "screens/ip/v4_utils.h"

#define V4_INFO_SEC_MAX 34
#define V4_RANGE_SEC_MAX 28
#define V4_HORIZONTAL_PADDING 2

#define V4_RANGE_SEC_TITLE 13

typedef struct {
    v4_cidr_t cidr;
    v4_field_t mask;
    v4_field_t network;
    v4_field_t broadcast;
    v4_field_t first;
    v4_field_t last;
    char hosts[TINY_BUFFER];
    char net_type[MINI_BUFFER];
} v4_info_t;

typedef struct {
    uint32_t address;
    uint32_t mask;
    uint8_t min_mask_prefix;
    const char *status;
} route_rule_t;

static void v4_buffer_clean(void);

static void v4_get_class(void);

static void render_v4_information(const int start_x, const int start_y);

static char v4_buf[V4_INPUT_MAX];

static input_field_t v4_field[] = {
    {.buf = v4_buf, .cap = sizeof(v4_buf)},
};

static v4_info_t v4;
static bool v4_has_result;
static bool v4_is_valid;
static route_rule_t v4_rules[] = {
    /* Specific single addresses */
    {0x00000000, 0xFFFFFFFF, 0, "N/A (Default Route)"},
    {0xFFFFFFFF, 0xFFFFFFFF, 0, "N/A (Limited Broadcast)"},

    /* Special ranges */
    {0x00000000, 0xFF000000, 1, "N/A (This Network)"},
    {0x64400000, 0xFFC00000, 0, "A (Shared / CGNAT)"},
    {0x7F000000, 0xFF000000, 0, "A (Loopback)"},
    {0xA9FE0000, 0xFFFF0000, 0, "B (APIPA (Link-Local))"},
    {0xC0000000, 0xFFFFFF00, 0, "C (IETF Protocol Assignments)"},
    {0xC0000200, 0xFFFFFF00, 0, "C (TEST-NET-1)"},
    {0xC6120000, 0xFFFE0000, 0, "C (Benchmarking)"},
    {0xC6336400, 0xFFFFFF00, 0, "C (TEST-NET-2)"},
    {0xCB007100, 0xFFFFFF00, 0, "C (TEST-NET-3)"},

    /* Private networks (Private / RFC 1918) */
    {0x0A000000, 0xFF000000, 0, "A (Private)"},
    {0xAC100000, 0xFFF00000, 0, "B (Private)"},
    {0xC0A80000, 0xFFFF0000, 0, "C (Private)"},

    /* Broadcasting and Experiments */
    {0xE0000000, 0xF0000000, 0, "D (Multicast)"},
    {0xF0000000, 0xF0000000, 0, "E (Experimental / Reversed)"},
};

static void v4_buffer_clean(void) { memset(&v4, 0, sizeof(v4)); }

static void v4_get_class(void) {
    for (int i = 0; i < (int)(sizeof(v4_rules) / sizeof(v4_rules[0])); i++) {
        if ((v4.cidr.base.value & v4_rules[i].mask) == v4_rules[i].address) {
            if (v4_rules[i].min_mask_prefix > 0 && v4.cidr.mask_prefix == 0)
                continue;

            memcpy(v4.net_type, v4_rules[i].status,
                   strlen(v4_rules[i].status) + 1);
            return;
        }
    }

    char *v4_net_type = NULL;

    if (v4.cidr.base.octets[0] <= 127) {
        v4_net_type = "A (Public)";
    } else if (v4.cidr.base.octets[0] <= 191) {
        v4_net_type = "B (Public)";
    } else {
        v4_net_type = "C (Public)";
    }

    memcpy(v4.net_type, v4_net_type, strlen(v4_net_type) + 1);
}

static void render_v4_information(const int start_x, const int start_y) {
    if (!v4_is_valid) {
        render_input_error(start_x, start_y + 1, v4_field->error_msg);
        return;
    }

    render_section_title(start_x, start_y + 1, "[ NETWORK INFORMATION ]");
    render_label_value_pair(start_x, start_y + 2, V4_INPUT_MAX,
                            "IP Address:", v4.cidr.base.string);
    render_label_value_pair(start_x, start_y + 3, V4_INPUT_MAX,
                            "Subnet Mask:", v4.mask.string);
    render_label_value_pair(start_x, start_y + 4, V4_INPUT_MAX,
                            "Network Address:", v4.network.string);
    render_label_value_pair(start_x, start_y + 5, V4_INPUT_MAX,
                            "Broadcast Address:", v4.broadcast.string);

    int needed_x =
        start_x + V4_INFO_SEC_MAX + V4_HORIZONTAL_PADDING + V4_RANGE_SEC_MAX;
    int window_x = sizes.win_x + sizes.win_w - 2;
    bool enough_size = needed_x <= window_x;

    int range_section_x =
        (enough_size) ? start_x + V4_INFO_SEC_MAX + V4_HORIZONTAL_PADDING
                      : start_x;
    int range_section_y = (enough_size) ? start_y + 1 : start_y + 7;
    int range_section_width = (enough_size) ? V4_RANGE_SEC_TITLE : V4_INPUT_MAX;

    render_section_title(range_section_x, range_section_y,
                         "[ RANGE OF NODES ]");
    render_label_value_pair(range_section_x, range_section_y + 1,
                            range_section_width,
                            "First Host:", v4.first.string);
    render_label_value_pair(range_section_x, range_section_y + 2,
                            range_section_width, "Last Host:", v4.last.string);
    render_label_value_pair(range_section_x, range_section_y + 3,
                            range_section_width, "Total Hosts:", v4.hosts);
    render_label_value_pair(range_section_x, range_section_y + 4,
                            range_section_width, "Class:", v4.net_type);

    int binary_section_y = (enough_size) ? start_y + 7 : range_section_y + 6;

    render_section_title(start_x, binary_section_y, "[ BINARY FORM ]");
    render_bin_section(start_x, binary_section_y + 1, V4_INPUT_MAX,
                       "IP:", v4.cidr.base.octets);
    render_bin_section(start_x, binary_section_y + 2, V4_INPUT_MAX,
                       "Mask:", v4.mask.octets);
}

void v4_focus_change(void) {
    app.active_field = &v4_field[0];
    app.active_accept = v4_input_accept;
    app.input_count = 1;
}

void v4_input_confirm(void) {
    v4_buffer_clean();
    v4_has_result = true;

    v4_parse_cidr(v4_field[0].buf, &v4.cidr, v4_field->error_msg,
                  sizeof(v4_field->error_msg));

    v4_is_valid = app.is_input_correct;
    if (!v4_is_valid)
        return;

    parse_mask(v4.cidr.mask_prefix, &v4.mask);

    calc_v4_network_address(v4.cidr.base.value, v4.mask.value, &v4.network);
    calc_v4_broadcast_address(v4.network.value, v4.mask.value, &v4.broadcast);

    calc_v4_capacity(v4.cidr.mask_prefix, v4.hosts, sizeof(v4.hosts));
    calc_v4_first_node(v4.cidr.mask_prefix, v4.network.value, &v4.first);
    calc_v4_last_node(v4.cidr.mask_prefix, v4.cidr.base.value,
                      v4.broadcast.value, &v4.last);

    v4_get_class();
}

void render_v4_content(const int start_x, const int start_y) {
    render_input_block(start_x, start_y, V4_INPUT_MAX, "IP:", 1, &v4_field[0],
                       "192.168.0.54/26", v4_validate, true);

    if (!v4_has_result)
        return;

    render_v4_information(start_x, start_y + 1);
}
