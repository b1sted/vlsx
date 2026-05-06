/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "termbox2.h"

#include "app.h"
#include "ui.h"

#include "screens/ip/v4.h"

#define V4_INFO_SEC_MAX 34
#define V4_RANGE_SEC_MAX 28
#define V4_HORIZONTAL_PADDING 2

#define V4_RANGE_SEC_TITLE 13

typedef enum {
    V4_C_DIGIT,
    V4_C_DOT,
    V4_C_SLASH,
    V4_C_OTHER,
    V4_NUM_CLASSES,
} V4Class;

typedef enum {
    V4_S_START,
    V4_S_OCTDIGIT,
    V4_S_DOT,
    V4_S_SLASH,
    V4_S_MSKDIGIT,
    V4_NUM_STATES,
} V4State;

typedef struct {
    uint32_t value;
    uint8_t octets[4];
    char string[TINY_BUFFER];
} v4_field_t;

typedef struct {
    v4_field_t address;
    v4_field_t mask;
    v4_field_t network;
    v4_field_t broadcast;
    v4_field_t first;
    v4_field_t last;
    char hosts[TINY_BUFFER];
    char net_type[MINI_BUFFER];
    uint8_t mask_prefix;
} v4_info_t;

typedef struct {
    uint32_t address;
    uint32_t mask;
    uint8_t min_mask_prefix;
    const char *status;
} route_rule_t;

static int ipv4_char_class(char ch);
static bool ipv4_accept(char ch);
static bool parse_ipv4(const char *str);

static void ipv4_buffer_clean(void);
static void uint32_to_octets(uint32_t v4_adr, uint8_t *octets_arr);
static void ipv4_to_str(const uint8_t *octets_arr, char *str, size_t buf_size);

static void parse_mask(void);
static void calc_ipv4_network_address(void);
static void calc_ipv4_broadcast_address(void);
static void calc_ipv4_node_count(void);
static void calc_ipv4_first_node(void);
static void calc_ipv4_last_node(void);
static void ipv4_get_class(void);

static void render_ipv4_information(const int start_x, const int start_y);

static const int ipv4_trans[V4_NUM_STATES][V4_NUM_CLASSES] = {
    /*              DIGIT          DOT    SLASH  OTHER */
    [V4_S_START] = {V4_S_OCTDIGIT, S_ERR, S_ERR, S_ERR},
    [V4_S_OCTDIGIT] = {V4_S_OCTDIGIT, V4_S_DOT, V4_S_SLASH, S_ERR},
    [V4_S_DOT] = {V4_S_OCTDIGIT, S_ERR, S_ERR, S_ERR},
    [V4_S_SLASH] = {V4_S_MSKDIGIT, S_ERR, S_ERR, S_ERR},
    [V4_S_MSKDIGIT] = {V4_S_MSKDIGIT, S_ERR, S_ERR, S_ERR},
};

static const bool ipv4_finals[V4_NUM_STATES] = {
    [V4_S_MSKDIGIT] = true,
};

static const dfa_t ipv4_validator = {
    .num_states = V4_NUM_STATES,
    .num_classes = V4_NUM_CLASSES,
    .start_state = V4_S_START,
    .transitions = (const int *)ipv4_trans,
    .is_final = ipv4_finals,
    .get_class = ipv4_char_class,
};

static char ipv4_buf[V4_INPUT_MAX];

static input_field_t ipv4_field[] = {
    {.buf = ipv4_buf, .cap = sizeof(ipv4_buf)},
};

static v4_info_t v4;
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

static int ipv4_char_class(char ch) {
    if (ch >= '0' && ch <= '9')
        return V4_C_DIGIT;
    if (ch == '.')
        return V4_C_DOT;
    if (ch == '/')
        return V4_C_SLASH;
    return V4_C_OTHER;
}

static bool ipv4_accept(char ch) {
    return ipv4_validator.get_class(ch) != V4_C_OTHER;
}

static bool parse_ipv4(const char *str) {
    int octets[4], mask, n;

    if (sscanf(str, "%d.%d.%d.%d/%d%n", &octets[0], &octets[1], &octets[2],
               &octets[3], &mask, &n) != 5 ||
        str[n] != '\0') {
        snprintf(ipv4_field->error_msg, sizeof(ipv4_field->error_msg),
                 "Invalid input.\nExpected format: A.B.C.D/mask");
        return false;
    }

    for (int i = 0; i < 4; i++) {
        if (octets[i] < 0 || octets[i] > 255) {
            snprintf(ipv4_field->error_msg, sizeof(ipv4_field->error_msg),
                     "Invalid input.\nThe value of octet %d is out of range: "
                     "[0, 255]",
                     i + 1);
            return false;
        }

        v4.address.value = (v4.address.value << 8) | (uint8_t)octets[i];
    }

    if (mask < 0 || mask > 32) {
        snprintf(ipv4_field->error_msg, sizeof(ipv4_field->error_msg),
                 "Invalid input.\nMask prefix %d is out of range: [0, 32]",
                 mask);
        return false;
    }

    v4.mask_prefix = (uint8_t)mask;

    uint32_to_octets(v4.address.value, v4.address.octets);
    ipv4_to_str(v4.address.octets, v4.address.string,
                sizeof(v4.address.string));

    return true;
}

static void ipv4_buffer_clean(void) { memset(&v4, 0, sizeof(v4)); }

static void uint32_to_octets(uint32_t v4_adr, uint8_t *octets_arr) {
    octets_arr[0] = (v4_adr >> 24) & 0xFF;
    octets_arr[1] = (v4_adr >> 16) & 0xFF;
    octets_arr[2] = (v4_adr >> 8) & 0xFF;
    octets_arr[3] = v4_adr;
}

static void ipv4_to_str(const uint8_t *octets_arr, char *str, size_t buf_size) {
    snprintf(str, buf_size, "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8,
             octets_arr[0], octets_arr[1], octets_arr[2], octets_arr[3]);
}

static void parse_mask(void) {
    v4.mask.value =
        (v4.mask_prefix == 0) ? 0 : (0xFFFFFFFFu << (32 - v4.mask_prefix));

    uint32_to_octets(v4.mask.value, v4.mask.octets);
    ipv4_to_str(v4.mask.octets, v4.mask.string, sizeof(v4.mask.string));
}

static void calc_ipv4_network_address(void) {
    if (v4.mask_prefix == 31) {
        memcpy(v4.network.string, "N/A (RFC 3021)", 15);
        return;
    }

    v4.network.value = v4.address.value & v4.mask.value;

    uint32_to_octets(v4.network.value, v4.network.octets);
    ipv4_to_str(v4.network.octets, v4.network.string,
                sizeof(v4.network.string));
}

static void calc_ipv4_broadcast_address(void) {
    if (v4.mask_prefix == 32) {
        memcpy(v4.broadcast.string, "N/A", 4);
        return;
    }

    if (v4.mask_prefix == 31) {
        memcpy(v4.broadcast.string, "N/A (RFC 3021)", 15);
        return;
    }

    v4.broadcast.value = v4.network.value | ~v4.mask.value;

    uint32_to_octets(v4.broadcast.value, v4.broadcast.octets);
    ipv4_to_str(v4.broadcast.octets, v4.broadcast.string,
                sizeof(v4.broadcast.string));
}

static void calc_ipv4_node_count(void) {
    if (v4.mask_prefix == 32) {
        snprintf(v4.hosts, sizeof(v4.hosts), "1");
        return;
    }

    if (v4.mask_prefix == 31) {
        snprintf(v4.hosts, sizeof(v4.hosts), "2");
        return;
    }

    uint64_t v4_node_count = (UINT64_C(1) << (32 - v4.mask_prefix)) - 2;

    snprintf(v4.hosts, sizeof(v4.hosts), "%" PRIu64 "", v4_node_count);
}

static void calc_ipv4_first_node(void) {
    if (v4.mask_prefix == 32 || v4.mask_prefix == 31) {
        memcpy(v4.first.string, v4.address.string,
               strlen(v4.address.string) + 1);
        return;
    }

    v4.first.value = v4.network.value + 1;

    uint32_to_octets(v4.first.value, v4.first.octets);
    ipv4_to_str(v4.first.octets, v4.first.string, sizeof(v4.first.string));
}

static void calc_ipv4_last_node(void) {
    if (v4.mask_prefix == 32) {
        memcpy(v4.last.string, v4.address.string,
               strlen(v4.address.string) + 1);
        return;
    }

    v4.last.value =
        (v4.mask_prefix == 31) ? v4.address.value + 1 : v4.broadcast.value - 1;

    uint32_to_octets(v4.last.value, v4.last.octets);
    ipv4_to_str(v4.last.octets, v4.last.string, sizeof(v4.last.string));
}

static void ipv4_get_class(void) {
    for (int i = 0; i < (int)(sizeof(v4_rules) / sizeof(v4_rules[0])); i++) {
        if ((v4.address.value & v4_rules[i].mask) == v4_rules[i].address) {
            if (v4_rules[i].min_mask_prefix > 0 && v4.mask_prefix == 0)
                continue;

            memcpy(v4.net_type, v4_rules[i].status,
                   strlen(v4_rules[i].status) + 1);
            return;
        }
    }

    char *v4_net_type = NULL;

    if (v4.address.octets[3] <= 127) {
        v4_net_type = "A (Public)";
    } else if (v4.address.octets[3] <= 191) {
        v4_net_type = "B (Public)";
    } else {
        v4_net_type = "C (Public)";
    }

    memcpy(v4.net_type, v4_net_type, strlen(v4_net_type) + 1);
}

static void render_ipv4_information(const int start_x, const int start_y) {
    if (!app.is_input_result_valid) {
        render_input_error(start_x, start_y + 1, ipv4_field->error_msg);
        return;
    }

    render_section_title(start_x, start_y + 1, "[ NETWORK INFORMATION ]");
    render_label_value_pair(start_x, start_y + 2, V4_INPUT_MAX,
                            "IP Address:", v4.address.string);
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
                       "IP:", v4.address.octets[0], v4.address.octets[1],
                       v4.address.octets[2], v4.address.octets[3]);
    render_bin_section(start_x, binary_section_y + 2, V4_INPUT_MAX,
                       "Mask:", v4.mask.octets[0], v4.mask.octets[1],
                       v4.mask.octets[2], v4.mask.octets[3]);
}

void ipv4_focus_change(void) {
    app.active_field = &ipv4_field[0];
    app.active_accept = ipv4_accept;
    app.input_count = 1;
}

void ipv4_input_confirm(void) {
    ipv4_buffer_clean();

    if (!run_dfa(&ipv4_validator, ipv4_field[0].buf)) {
        strncpy(ipv4_field->error_msg,
                "Invalid input.\nPlease correct it and try again.",
                sizeof(ipv4_field->error_msg));
        app.is_input_result_valid = false;
        return;
    }

    if (!parse_ipv4(ipv4_field[0].buf)) {
        app.is_input_result_valid = false;
        return;
    }

    app.is_input_result_valid = true;

    parse_mask();
    calc_ipv4_network_address();
    calc_ipv4_broadcast_address();

    calc_ipv4_node_count();
    calc_ipv4_first_node();
    calc_ipv4_last_node();

    ipv4_get_class();
}

void render_ipv4_content(const int start_x, const int start_y) {
    const char *label = "IP:";
    const int label_len = (int)strlen(label);

    tb_print(start_x, start_y, COLOR_BLACK | TB_BOLD, COLOR_SILVER, label);

    int input_x = start_x + label_len + 1;
    render_input_field(input_x, start_y, &ipv4_field[0], "192.168.0.54/26");
    render_input_status(input_x + V4_INPUT_MAX, start_y, ipv4_field,
                        &ipv4_validator);

    if (!app.has_press_input)
        return;

    render_ipv4_information(start_x, start_y + 1);
}
