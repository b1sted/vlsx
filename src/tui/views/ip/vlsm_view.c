/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/ip/vlsm_view.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common/limits.h"
#include "core/net/ipv4/ipv4_calc.h"
#include "core/net/ipv4/ipv4_validate.h"
#include "core/net/vlsm/hosts_list.h"
#include "core/net/vlsm/vlsm_types.h"
#include "screens/ip/vlsm_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"
#include "tui/table.h"

#define VLSM_COL_COUNT 6
#define VLSM_TABLE_HEADER_ROWS 2
#define VLSM_TABLE_INFO_ROWS 4
#define VLSM_TABLE_MIN_DATA_ROWS 7
#define VLSM_TABLE_MIN_VIEW_WIDTH 54
#define VLSM_TABLE_MIN_VIEW_HEIGHT (VLSM_TABLE_HEADER_ROWS + VLSM_TABLE_MIN_DATA_ROWS)
#define VLSM_INFO_LABEL_WIDTH 14
#define VLSM_INPUT_LABEL_WIDTH 6

typedef enum {
    VLSM_COL_REQ,
    VLSM_COL_ALLOC,
    VLSM_COL_PREFIX,
    VLSM_COL_NETWORK,
    VLSM_COL_BROADCAST,
    VLSM_COL_RANGE,
} vlsm_column_t;

static const table_column_t vlsm_columns[VLSM_COL_COUNT] = {
    {"Req", 5, TABLE_ALIGN_LEFT},
    {" Alloc ", 7, TABLE_ALIGN_LEFT},
    {" Pfx ", 5, TABLE_ALIGN_LEFT},
    {" Network Addr", 18, TABLE_ALIGN_LEFT},
    {" Broadcast Addr", 18, TABLE_ALIGN_LEFT},
    {" Range", 19, TABLE_ALIGN_LEFT},
};

static char vlsm_cells[VLSM_MAX_SUBNETS][VLSM_COL_COUNT][SMALL_BUFFER];
static const char *vlsm_cell_ptrs[VLSM_MAX_SUBNETS * VLSM_COL_COUNT];

static int vlsm_view_row_offset = 0;
static int vlsm_view_column_offset = 0;
static int vlsm_view_width = 0;
static int vlsm_view_height = 0;
static int vlsm_view_row_count = 0;
static unsigned vlsm_view_generation = 0;
static bool vlsm_view_initialized = false;

static int vlsm_view_clamp(int value, int min, int max);
static void vlsm_view_build_cells(const vlsm_result_t *result);
static table_t vlsm_view_make_table(const vlsm_screen_t *screen);
static void vlsm_view_render_information(const vlsm_screen_t *screen, int x, int y);
static void vlsm_view_render_table(const vlsm_screen_t *screen, const layout_t *layout);

static int vlsm_view_clamp(int value, int min, int max)
{
    if (max < min) {
        return min;
    }

    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

static void vlsm_view_build_cells(const vlsm_result_t *result)
{
    if (result == NULL) {
        return;
    }

    const size_t row_count =
        (result->entry_count > VLSM_MAX_SUBNETS) ? VLSM_MAX_SUBNETS : result->entry_count;

    for (size_t i = 0; i < row_count; i++) {
        const vlsm_entry_t *entry = &result->entries[i];
        char network[V4_INPUT_MAX];
        char broadcast[V4_INPUT_MAX];

        ipv4_format_addr(&entry->network, network, sizeof(network));
        ipv4_format_addr(&entry->broadcast, broadcast, sizeof(broadcast));

        snprintf(vlsm_cells[i][VLSM_COL_REQ], SMALL_BUFFER, " %" PRIu32, entry->requested_hosts);
        snprintf(vlsm_cells[i][VLSM_COL_ALLOC], SMALL_BUFFER, "  %" PRIu32, entry->allocated_hosts);
        snprintf(vlsm_cells[i][VLSM_COL_PREFIX], SMALL_BUFFER, " /%u", (unsigned)entry->prefix);
        snprintf(vlsm_cells[i][VLSM_COL_NETWORK], SMALL_BUFFER, " %s", network);
        snprintf(vlsm_cells[i][VLSM_COL_BROADCAST], SMALL_BUFFER, " %s", broadcast);
        snprintf(vlsm_cells[i][VLSM_COL_RANGE], SMALL_BUFFER, " .%u.%u - .%u.%u",
                 (unsigned)entry->first.octets[2], (unsigned)entry->first.octets[3],
                 (unsigned)entry->last.octets[2], (unsigned)entry->last.octets[3]);

        for (size_t column = 0; column < VLSM_COL_COUNT; column++) {
            vlsm_cell_ptrs[i * VLSM_COL_COUNT + column] = vlsm_cells[i][column];
        }
    }
}

static table_t vlsm_view_make_table(const vlsm_screen_t *screen)
{
    table_t table = {
        .columns = vlsm_columns,
        .column_count = VLSM_COL_COUNT,
        .cells = vlsm_cell_ptrs,
        .row_count = 0,
    };

    if (screen != NULL && screen->is_valid) {
        table.row_count = (int)screen->result.entry_count;
    }

    return table;
}

static void vlsm_view_render_information(const vlsm_screen_t *screen, int x, int y)
{
    char base_network_addr[V4_INPUT_MAX];
    char base_network[SMALL_BUFFER];
    char allocated_hosts[MINI_BUFFER];
    char unallocated_hosts[MINI_BUFFER];

    ipv4_format_addr(&screen->result.base_network, base_network_addr, sizeof(base_network_addr));

    snprintf(base_network, sizeof(base_network), "%s/%u (Capacity: %" PRIu32 " hosts)",
             base_network_addr, (unsigned)screen->result.base_prefix, screen->result.capacity);
    snprintf(allocated_hosts, sizeof(allocated_hosts), "%" PRIu32 " hosts (%.2f%% used)",
             screen->result.allocated_hosts, screen->result.percent_usage);
    snprintf(unallocated_hosts, sizeof(unallocated_hosts), "%" PRIu32 " hosts",
             screen->result.unallocated_hosts);

    ui_render_section_title(x, y, "[ UTILIZATION SUMMARY ]");
    ui_render_label_value_pair(x, y + 1, VLSM_INFO_LABEL_WIDTH, "Base Network:", base_network);
    ui_render_label_value_pair(x, y + 2, VLSM_INFO_LABEL_WIDTH, "Allocated:", allocated_hosts);
    ui_render_label_value_pair(x, y + 3, VLSM_INFO_LABEL_WIDTH, "Unallocated:", unallocated_hosts);
}

static void vlsm_view_render_table(const vlsm_screen_t *screen, const layout_t *layout)
{
    const int table_y = layout->content_y + 4;
    const int table_right = layout->win_x + layout->win_w - 1;
    const int table_bottom = layout->win_y + layout->win_h - 1;
    const int table_width = table_right - layout->content_x;
    const int available_height = table_bottom - table_y;
    const int table_height = available_height - VLSM_TABLE_INFO_ROWS;

    if (table_width < VLSM_TABLE_MIN_VIEW_WIDTH || table_height < VLSM_TABLE_MIN_VIEW_HEIGHT) {
        return;
    }

    vlsm_view_width = table_width;
    vlsm_view_height = table_height;
    vlsm_view_row_count = (int)screen->result.entry_count;

    vlsm_view_build_cells(&screen->result);

    table_t table = vlsm_view_make_table(screen);
    const int info_y = table_y + table_total_height(&table, table_height);

    vlsm_view_row_offset =
        vlsm_view_clamp(vlsm_view_row_offset, 0, table_max_row_offset(&table, table_height));
    vlsm_view_column_offset =
        vlsm_view_clamp(vlsm_view_column_offset, 0, table_max_column_offset(&table, table_width));

    table_render(layout->content_x, table_y, table_width, table_height, &table,
                 vlsm_view_row_offset, vlsm_view_column_offset);

    vlsm_view_render_information(screen, layout->content_x, info_y + 1);
}

void vlsm_view_render(const vlsm_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    if (!vlsm_view_initialized || screen->generation != vlsm_view_generation) {
        vlsm_view_row_offset = 0;
        vlsm_view_column_offset = 0;
        vlsm_view_generation = screen->generation;
        vlsm_view_initialized = true;
    }

    const bool first_active = (screen->active_input == 0);

    ui_render_input_block(layout->content_x, layout->content_y, MINI_BUFFER, VLSM_INPUT_LABEL_WIDTH,
                          "CIDR:", &screen->fields[0], "192.168.0.54/26", ipv4_validate,
                          first_active);
    ui_render_input_block(layout->content_x, layout->content_y + 1, MINI_BUFFER,
                          VLSM_INPUT_LABEL_WIDTH, "Hosts:", &screen->fields[1], "500, 100, 2",
                          hosts_validate, !first_active);

    if (!screen->has_result) {
        return;
    }

    if (!screen->is_valid) {
        ui_render_input_error(layout->content_x, layout->content_y + 3, screen->status.message);
        return;
    }

    ui_render_section_title(layout->content_x, layout->content_y + 3, "[ VLSM SUBNETING PLAN ]");
    vlsm_view_render_table(screen, layout);
}

bool vlsm_view_handle_event(const vlsm_screen_t *screen, const ui_event_t *event)
{
    if (screen == NULL || event == NULL) {
        return false;
    }

    if (!screen->has_result || !screen->is_valid || screen->result.entry_count == 0 ||
        vlsm_view_row_count <= 0 || vlsm_view_height <= 0 || vlsm_view_width <= 0) {
        return false;
    }

    table_t table = vlsm_view_make_table(screen);
    const int max_row = table_max_row_offset(&table, vlsm_view_height);
    const int max_column = table_max_column_offset(&table, vlsm_view_width);

    if (event->kind == UI_EVENT_KEY_CHAR && event->ch == (uint32_t)'j') {
        vlsm_view_row_offset = vlsm_view_clamp(vlsm_view_row_offset + 1, 0, max_row);
        return true;
    }

    if (event->kind == UI_EVENT_KEY_CHAR && event->ch == (uint32_t)'k') {
        vlsm_view_row_offset = vlsm_view_clamp(vlsm_view_row_offset - 1, 0, max_row);
        return true;
    }

    if (event->kind == UI_EVENT_KEY_CHAR && event->ch == (uint32_t)'l') {
        vlsm_view_column_offset = vlsm_view_clamp(vlsm_view_column_offset + 1, 0, max_column);
        return true;
    }

    if (event->kind == UI_EVENT_KEY_CHAR && event->ch == (uint32_t)'h') {
        vlsm_view_column_offset = vlsm_view_clamp(vlsm_view_column_offset - 1, 0, max_column);
        return true;
    }

    return false;
}
