/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "termbox2.h"

#include "app.h"
#include "dfa.h"
#include "input.h"
#include "render.h"
#include "ui.h"
#include "utils.h"

#include "screens/ip/v4_utils.h"
#include "screens/ip/vlsm.h"

#define MAX_SUBNETS 16
#define VLSM_COL_COUNT 6

/* Minimum supported terminal is 90x24. These values are minimum guarantees,
 * not caps: on larger terminals the viewport expands to fill the available
 * content area while keeping VLSM_TABLE_INFO_ROWS free below the table. */
#define VLSM_TABLE_HEADER_ROWS 2
#define VLSM_TABLE_INFO_ROWS 4
#define VLSM_INFO_LABEL_WIDTH 14
#define VLSM_UTILIZATION_COLUMN_OFFSET 32
#define VLSM_UTILIZATION_LABEL_WIDTH 13
#define VLSM_TABLE_MIN_DATA_ROWS 7
#define VLSM_TABLE_MIN_VIEW_WIDTH 54
#define VLSM_TABLE_MIN_VIEW_HEIGHT                                           \
    (VLSM_TABLE_HEADER_ROWS + VLSM_TABLE_MIN_DATA_ROWS)

typedef enum {
    VLSM_COL_REQ,
    VLSM_COL_ALLOC,
    VLSM_COL_PREFIX,
    VLSM_COL_NETWORK,
    VLSM_COL_BROADCAST,
    VLSM_COL_RANGE,
} vlsm_column_t;

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

typedef struct {
    v4_field_t base_network;
    uint8_t base_prefix;
    uint32_t base_capacity;
    uint32_t allocated_hosts;
    uint32_t unallocated_hosts;
    float percent_usage;
} hosts_statistics_t;

typedef struct {
    v4_field_t first;
    v4_field_t last;
    char string[SMALL_BUFFER];
} vlsm_range_t;

typedef struct {
    vlsm_range_t address_range;
    v4_field_t network_address;
    v4_field_t broadcast_address;
    uint32_t requested_hosts;
    uint32_t allocated_hosts;
    uint32_t unused_hosts;
    char mask_prefix[MICRO_BUFFER];
} vlsm_entry_t;

static void vlsm_buffer_clean(void);

static int hosts_char_class(char ch);
static bool hosts_input_accept(char ch);
static bool hosts_validate(const char *input);

static int compare_desc(const void *a, const void *b);
static void sort_descending(uint32_t *array, size_t array_items);

static uint32_t parse_hosts(const char *input, uint32_t *hosts_arr,
                            uint8_t *array_items, char *error_buf,
                            size_t buf_size);

static void vlsm_table_completion(const uint32_t base_address,
                                  const uint8_t mask_prefix,
                                  vlsm_entry_t *table, uint32_t *hosts_array,
                                  uint8_t array_items);
static void hosts_calc_usage(hosts_statistics_t *stats);

static void vlsm_table_cells_build(const int row_count);
static table_t vlsm_table_view(void);
static void render_vlsm_information(const int start_x, const int start_y);
static void render_vlsm_table(const int start_x, const int start_y);
static int vlsm_clamp(const int value, const int min, const int max);

static const int hosts_trans[HOSTS_NUM_STATES][HOSTS_NUM_CLASSES] = {
    /*                 DIGIT          COMMA  SPACE  OTHER */
    [HOSTS_S_START] = {HOSTS_S_DIGIT, S_ERR, S_ERR, S_ERR},
    [HOSTS_S_DIGIT] = {HOSTS_S_DIGIT, HOSTS_S_COMMA, S_ERR, S_ERR},
    [HOSTS_S_COMMA] = {HOSTS_S_DIGIT, S_ERR, HOSTS_S_SPACE, S_ERR},
    [HOSTS_S_SPACE] = {HOSTS_S_DIGIT, S_ERR, S_ERR, S_ERR},
};

static const bool hosts_finals[HOSTS_NUM_STATES] = {
    [HOSTS_S_DIGIT] = true,
};

static const dfa_t hosts_validator = {
    .num_states = HOSTS_NUM_STATES,
    .num_classes = HOSTS_NUM_CLASSES,
    .start_state = HOSTS_S_START,
    .transitions = (const int *)hosts_trans,
    .is_final = hosts_finals,
    .get_class = hosts_char_class,
};

static char vlsm_cidr_buf[V4_INPUT_MAX];
static char vlsm_array_buf[MINI_BUFFER];

static input_field_t vlsm_field[] = {
    {.buf = vlsm_cidr_buf, .cap = sizeof(vlsm_cidr_buf)},
    {.buf = vlsm_array_buf, .cap = sizeof(vlsm_array_buf)},
};

static const table_column_t vlsm_columns[VLSM_COL_COUNT] = {
    {"Req", 5, TABLE_ALIGN_LEFT},
    {" Alloc ", 7, TABLE_ALIGN_LEFT},
    {" Pfx ", 5, TABLE_ALIGN_LEFT},
    {" Network Addr", 18, TABLE_ALIGN_LEFT},
    {" Broadcast Addr", 18, TABLE_ALIGN_LEFT},
    {" Range", 19, TABLE_ALIGN_LEFT},
};

static v4_cidr_t vlsm_cidr;
static vlsm_entry_t vlsm_table[MAX_SUBNETS];
static uint32_t num_requests[MAX_SUBNETS];
static hosts_statistics_t hosts_statistics;
static char error_msg[MEDIUM_BUFFER];
static bool vlsm_has_result;
static bool vlsm_is_valid;

static char vlsm_cells[MAX_SUBNETS][VLSM_COL_COUNT][SMALL_BUFFER];
static const char *vlsm_cell_ptrs[MAX_SUBNETS * VLSM_COL_COUNT];

static int vlsm_row_count = 0;
static int vlsm_row_offset = 0;
static int vlsm_column_offset = 0;
static int vlsm_view_width = 0;
static int vlsm_view_height = 0;
static int vlsm_info_y = 0;

static void vlsm_buffer_clean(void) {
    memset(&vlsm_cidr, 0, sizeof(vlsm_cidr));
    memset(vlsm_table, 0, sizeof(vlsm_table));
    memset(num_requests, 0, sizeof(num_requests));
    memset(error_msg, 0, sizeof(error_msg));
    memset(&hosts_statistics, 0, sizeof(hosts_statistics));

    vlsm_row_count = 0;
    vlsm_row_offset = 0;
    vlsm_column_offset = 0;
    vlsm_info_y = 0;
}

static int hosts_char_class(char ch) {
    if (ch >= '0' && ch <= '9')
        return HOSTS_C_DIGIT;
    if (ch == ',')
        return HOSTS_C_COMMA;
    if (ch == ' ')
        return HOSTS_C_SPACE;
    return HOSTS_C_OTHER;
}

static bool hosts_input_accept(char ch) {
    return hosts_validator.get_class(ch) != HOSTS_C_OTHER;
}

static bool hosts_validate(const char *input) {
    return run_dfa(&hosts_validator, input);
}

static int compare_desc(const void *a, const void *b) {
    uint32_t arg1 = *(const uint32_t *)a;
    uint32_t arg2 = *(const uint32_t *)b;

    if (arg1 > arg2)
        return -1;
    if (arg1 < arg2)
        return 1;
    return 0;
}

static void sort_descending(uint32_t *array, size_t array_items) {
    qsort(array, array_items, sizeof(uint32_t), compare_desc);
}

static uint32_t parse_hosts(const char *input, uint32_t *hosts_arr,
                            uint8_t *array_items, char *error_buf,
                            size_t buf_size) {
    char err_msg[SMALL_BUFFER] = "";
    size_t offset = strlen(error_buf);

    if (!run_dfa(&hosts_validator, input)) {
        memcpy(err_msg, "Invalid input.\nPlease correct it and try again.",
               sizeof("Invalid input.\nPlease correct it and try again."));
        goto fail;
    }

    char *copy = strdup(input);
    char *endptr;
    char *token = strtok(copy, " ,");
    uint32_t needed_hosts = 0;

    while (token != NULL && *array_items < MAX_SUBNETS) {
        uint64_t val = strtoull(token, &endptr, 10);

        if (val == 0) {
            token = strtok(NULL, " ,");
            continue;
        }

        if (val > 4294967294ULL) {
            memcpy(err_msg, "Invalid input.\nHost value too large for IPv4.",
                   sizeof("Invalid input.\nHost value too large for IPv4."));
            free(copy);
            goto fail;
        }

        hosts_arr[*array_items] = (uint32_t)val;
        (*array_items)++;

        if (needed_hosts > UINT32_MAX - val) {
            memcpy(err_msg,
                   "Invalid input.\nRequested hosts exceed total IPv4 address "
                   "space.",
                   sizeof("Invalid input.\nRequested hosts exceed total IPv4 "
                          "address space."));
            free(copy);
            goto fail;
        }

        needed_hosts += (uint32_t)val;

        token = strtok(NULL, " ,");
    }

    app.is_input_correct = true;
    free(copy);
    return needed_hosts;

fail:
    snprintf(error_buf + offset, buf_size - offset, "%s%s",
             (offset == 0) ? "" : "\n\n", err_msg);
    app.is_input_correct = false;
    return 0;
}

static void vlsm_table_completion(const uint32_t base_address,
                                  const uint8_t mask_prefix,
                                  vlsm_entry_t *table, uint32_t *hosts_array,
                                  uint8_t array_items) {
    uint32_t base = base_address;
    uint8_t mask = mask_prefix;
    const uint32_t parent_last =
        base_address + ((UINT32_C(1) << (32 - mask_prefix)) - 1);

    for (uint8_t i = 0; i < array_items; i++) {
        uint32_t provisioned_hosts = round_up_pow2(hosts_array[i] + 2);
        if (provisioned_hosts < hosts_array[i]) {
            memcpy(error_msg,
                   "VLSM allocation failed.\nRequired subnet size exceeds the "
                   "remaining network capacity.",
                   sizeof("VLSM allocation failed.\nRequired subnet size "
                          "exceeds the remaining network capacity."));
            goto fail;
        }

        uint8_t allocated_prefix = 32 - get_pow2_exponent(provisioned_hosts);
        if (allocated_prefix < mask) {
            memcpy(error_msg,
                   "Internal VLSM state corrupted.\nSubnets are not in sorted "
                   "order.",
                   sizeof("Internal VLSM state corrupted.\nSubnets are not in "
                          "sorted order."));
            goto fail;
        }

        if ((uint64_t)base + provisioned_hosts - 1 > parent_last) {
            memcpy(error_msg,
                   "VLSM allocation failed.\nRequired subnet size exceeds the "
                   "remaining network capacity.",
                   sizeof("VLSM allocation failed.\nRequired subnet size "
                          "exceeds the remaining network capacity."));
            goto fail;
        }

        table[i].requested_hosts = hosts_array[i];
        table[i].allocated_hosts = provisioned_hosts - 2;
        table[i].unused_hosts = table[i].allocated_hosts - hosts_array[i];

        hosts_statistics.allocated_hosts += table[i].allocated_hosts;

        v4_field_t subnet_mask = {0, {0, 0, 0, 0}, ""};
        parse_mask(allocated_prefix, &subnet_mask);

        calc_v4_broadcast_address(base, subnet_mask.value,
                                  &table[i].broadcast_address);

        calc_v4_first_node(allocated_prefix, base,
                           &table[i].address_range.first);
        calc_v4_last_node(allocated_prefix, base,
                          table[i].broadcast_address.value,
                          &table[i].address_range.last);
        string_combine(table[i].address_range.first.string, " - ",
                       table[i].address_range.last.string,
                       table[i].address_range.string,
                       sizeof(table[i].address_range.string));

        table[i].network_address.value = base;
        uint32_to_octets(table[i].network_address.value,
                         table[i].network_address.octets);
        v4_to_str(table[i].network_address.octets,
                  table[i].network_address.string,
                  sizeof(table[i].network_address.string));

        uint8_itoa(allocated_prefix, table[i].mask_prefix);
        table[i].mask_prefix[0] = '/';

        base = table[i].broadcast_address.value + 1;
        mask = allocated_prefix;
    }

    return;

fail:
    app.is_input_correct = false;
    return;
}

static int vlsm_clamp(const int value, const int min, const int max) {
    if (max < min)
        return min;
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

static void vlsm_table_cells_build(const int row_count) {
    const int rows = (row_count > MAX_SUBNETS) ? MAX_SUBNETS : row_count;

    for (int i = 0; i < rows; i++) {
        const vlsm_entry_t *entry = &vlsm_table[i];

        snprintf(vlsm_cells[i][VLSM_COL_REQ], SMALL_BUFFER, " %" PRIu32,
                 entry->requested_hosts);
        snprintf(vlsm_cells[i][VLSM_COL_ALLOC], SMALL_BUFFER, "  %" PRIu32,
                 entry->allocated_hosts);
        snprintf(vlsm_cells[i][VLSM_COL_PREFIX], SMALL_BUFFER, " %s",
                 entry->mask_prefix);
        snprintf(vlsm_cells[i][VLSM_COL_NETWORK], SMALL_BUFFER, " %s",
                 entry->network_address.string);
        snprintf(vlsm_cells[i][VLSM_COL_BROADCAST], SMALL_BUFFER, " %s",
                 entry->broadcast_address.string);
        snprintf(vlsm_cells[i][VLSM_COL_RANGE], SMALL_BUFFER,
                 " .%u.%u - .%u.%u",
                 (unsigned)entry->address_range.first.octets[2],
                 (unsigned)entry->address_range.first.octets[3],
                 (unsigned)entry->address_range.last.octets[2],
                 (unsigned)entry->address_range.last.octets[3]);

        for (int column = 0; column < VLSM_COL_COUNT; column++) {
            vlsm_cell_ptrs[i * VLSM_COL_COUNT + column] =
                vlsm_cells[i][column];
        }
    }
}

static table_t vlsm_table_view(void) {
    table_t table = {
        .columns = vlsm_columns,
        .column_count = VLSM_COL_COUNT,
        .cells = vlsm_cell_ptrs,
        .row_count = vlsm_row_count,
    };

    return table;
}

static void render_vlsm_information(const int start_x, const int start_y) {
    if (!vlsm_is_valid) {
        render_input_error(start_x, start_y, error_msg);
        return;
    }

    render_section_title(start_x, start_y, "[ UTILIZATION SUMMARY ]");

    char base_network[SMALL_BUFFER];
    char allocated_hosts[MINI_BUFFER];
    char unallocated_hosts[MINI_BUFFER];

    uint32_t capacity =
        hosts_statistics.allocated_hosts +
        hosts_statistics.unallocated_hosts;

    snprintf(base_network, sizeof(base_network),
             "%s/%u (Capacity: %" PRIu32 " hosts)",
             hosts_statistics.base_network.string,
             (unsigned)hosts_statistics.base_prefix,
             capacity);
    snprintf(allocated_hosts, sizeof(allocated_hosts),
             "%" PRIu32 " hosts (%.2f%% used)",
             hosts_statistics.allocated_hosts,
             hosts_statistics.percent_usage);
    snprintf(unallocated_hosts, sizeof(unallocated_hosts),
             "%" PRIu32 " hosts",
             hosts_statistics.unallocated_hosts);

    render_label_value_pair(start_x, start_y + 1, VLSM_INFO_LABEL_WIDTH,
                            "Base Network:", base_network);
    render_label_value_pair(start_x, start_y + 2, VLSM_INFO_LABEL_WIDTH,
                            "Allocated:", allocated_hosts);
    render_label_value_pair(start_x, start_y + 3, VLSM_INFO_LABEL_WIDTH,
                            "Unallocated:", unallocated_hosts);
}

static void render_vlsm_table(const int start_x, const int start_y) {
    const int table_y = start_y + 2;
    const int table_right = sizes.win_x + sizes.win_w - 1;
    const int table_bottom = sizes.win_y + sizes.win_h - 1;
    const int table_width = table_right - start_x;
    const int available_height = table_bottom - table_y;
    const int table_height = available_height - VLSM_TABLE_INFO_ROWS;

    if (table_width < VLSM_TABLE_MIN_VIEW_WIDTH ||
        table_height < VLSM_TABLE_MIN_VIEW_HEIGHT)
        return;

    vlsm_view_width = table_width;
    vlsm_view_height = table_height;

    table_t table = vlsm_table_view();
    vlsm_info_y = table_y + table_total_height(&table, table_height);

    vlsm_row_offset = vlsm_clamp(vlsm_row_offset, 0,
                                 table_max_row_offset(&table, table_height));
    vlsm_column_offset =
        vlsm_clamp(vlsm_column_offset, 0,
                   table_max_column_offset(&table, table_width));

    render_table(start_x, table_y, table_width, table_height, &table,
                 vlsm_row_offset, vlsm_column_offset);
}

void vlsm_focus_change(void) {
    app.active_field = &vlsm_field[app.input_cursor];
    app.active_accept =
        (app.input_cursor == 0) ? v4_input_accept : hosts_input_accept;
    app.input_count = 2;
}

static void hosts_calc_usage(hosts_statistics_t *stats) {
    stats->unallocated_hosts =
        (stats->base_capacity > stats->allocated_hosts)
            ? stats->base_capacity - stats->allocated_hosts
            : 0;
    stats->percent_usage =
        (stats->base_capacity > 0)
            ? ((float)stats->allocated_hosts / stats->base_capacity) * 100
            : 0;
}

void vlsm_input_confirm(void) {
    vlsm_buffer_clean();
    vlsm_has_result = true;

    v4_parse_cidr(vlsm_field[0].buf, &vlsm_cidr, error_msg, sizeof(error_msg));
    bool cidr_ok = app.is_input_correct;
    if (cidr_ok && vlsm_cidr.mask_prefix == 0) {
        memcpy(
            error_msg,
            "Invalid parent mask.\nPrefix /0 is not allowed for VLSM.",
            sizeof("Invalid parent mask.\nPrefix /0 is not allowed for VLSM."));
        app.is_input_correct = false;
        cidr_ok = false;
    }

    uint8_t array_items = 0;
    uint32_t requested_hosts =
        parse_hosts(vlsm_field[1].buf, num_requests, &array_items, error_msg,
                    sizeof(error_msg));
    bool hosts_ok = app.is_input_correct;

    if (!cidr_ok || !hosts_ok)
        return;

    hosts_statistics.base_capacity =
        calc_v4_capacity(vlsm_cidr.mask_prefix, NULL, 0);
    if (hosts_statistics.base_capacity < requested_hosts) {
        memcpy(error_msg,
               "Invalid input.\nRequested hosts exceed capacity of the subnet "
               "mask.",
               sizeof("Invalid input.\nRequested hosts exceed capacity of the "
                      "subnet mask."));
        app.is_input_correct = false;
        return;
    }

    sort_descending(num_requests, array_items);

    v4_field_t base_mask = {0, {0, 0, 0, 0}, ""};
    v4_field_t base_network = {0, {0, 0, 0, 0}, ""};
    parse_mask(vlsm_cidr.mask_prefix, &base_mask);
    calc_v4_network_address(vlsm_cidr.base.value, base_mask.value,
                            &base_network);

    hosts_statistics.base_network = base_network;
    hosts_statistics.base_prefix = vlsm_cidr.mask_prefix;

    vlsm_table_completion(base_network.value, vlsm_cidr.mask_prefix, vlsm_table,
                          num_requests, array_items);

    if (!app.is_input_correct)
        return;

    vlsm_row_count = array_items;
    vlsm_table_cells_build(vlsm_row_count);

    hosts_calc_usage(&hosts_statistics);
    vlsm_is_valid = true;
}

void render_vlsm_content(const int start_x, const int start_y) {
    bool is_first_active = (app.input_cursor == 0) ? true : false;

    render_input_block(start_x, start_y, MINI_BUFFER, "CIDR:", 2,
                       &vlsm_field[0], "192.168.0.54/26", v4_validate,
                       is_first_active);
    render_input_block(start_x, start_y + 1, MINI_BUFFER, "Hosts:", 1,
                       &vlsm_field[1], "500, 100, 2", hosts_validate,
                       !is_first_active);

    if (!vlsm_has_result)
        return;

    if (!vlsm_is_valid) {
        render_vlsm_information(start_x, start_y + 3);
        return;
    }

    render_section_title(start_x, start_y + 3, "[ VLSM SUBNETING PLAN ]");
    render_vlsm_table(start_x, start_y + 2);

    /* Renders at the first line reserved below the table. */
    render_vlsm_information(start_x, vlsm_info_y + 1);
}

bool vlsm_handle_event(struct tb_event *ev) {
    if (ev == NULL || !vlsm_has_result || !vlsm_is_valid ||
        vlsm_row_count <= 0)
        return false;

    table_t table = vlsm_table_view();
    const int max_row = table_max_row_offset(&table, vlsm_view_height);
    const int max_column = table_max_column_offset(&table, vlsm_view_width);

    if (ev->key == TB_KEY_ARROW_DOWN || ev->ch == 'j') {
        vlsm_row_offset = vlsm_clamp(vlsm_row_offset + 1, 0, max_row);
        return true;
    }

    if (ev->key == TB_KEY_ARROW_UP || ev->ch == 'k') {
        vlsm_row_offset = vlsm_clamp(vlsm_row_offset - 1, 0, max_row);
        return true;
    }

    if (ev->key == TB_KEY_ARROW_RIGHT || ev->ch == 'l') {
        vlsm_column_offset =
            vlsm_clamp(vlsm_column_offset + 1, 0, max_column);
        return true;
    }

    if (ev->key == TB_KEY_ARROW_LEFT || ev->ch == 'h') {
        vlsm_column_offset =
            vlsm_clamp(vlsm_column_offset - 1, 0, max_column);
        return true;
    }

    return false;
}
