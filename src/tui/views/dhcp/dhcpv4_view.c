/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/dhcp/dhcpv4_view.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "termbox2.h"

#include "common/limits.h"
#include "core/net/dhcp/dhcp_calc.h"
#include "screens/dhcp/dhcpv4_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"
#include "tui/theme.h"

#define DHCP_LABEL_WIDTH 25
#define DHCP_INPUT_OFFSET 12
#define DHCP_STATUS_OFFSET 35
#define DHCP_PLACEHOLDER "600"
#define DHCP_PLACEHOLDER_LEN 3

static void dhcpv4_view_render_input(const dhcpv4_screen_t *screen, const layout_t *layout);
static void dhcpv4_view_format_duration(double minutes, char *dest, size_t dest_size);
static int dhcpv4_view_render_detail(int x, int y, int content_width, const char *label,
                                     const char *value);

static void dhcpv4_view_render_input(const dhcpv4_screen_t *screen, const layout_t *layout)
{
    const input_field_t *field = &screen->input;
    const bool has_input = field->len > 0;
    const int input_x = layout->content_x + DHCP_INPUT_OFFSET;
    const int display_len = has_input ? (int)field->len : DHCP_PLACEHOLDER_LEN;

    tb_print(layout->content_x, layout->content_y, TUI_COLOR_BLACK | TB_BOLD, TUI_COLOR_SILVER,
             "Lease Time:");
    tb_print(input_x, layout->content_y, has_input ? TUI_COLOR_BLACK : TUI_COLOR_COBALT,
             TUI_COLOR_SILVER, has_input ? field->buf : DHCP_PLACEHOLDER);
    tb_print(input_x + display_len + 1, layout->content_y, TUI_COLOR_BLACK, TUI_COLOR_SILVER,
             "minutes");

    ui_render_input_status(layout->content_x + DHCP_STATUS_OFFSET, layout->content_y, field,
                           dhcp_validate_lease);

    ui_render_input_cursor(input_x, layout->content_y, field, DHCP_PLACEHOLDER);
}

static void dhcpv4_view_format_duration(double minutes, char *dest, size_t dest_size)
{
    if (dest == NULL || dest_size == 0) {
        return;
    }

    snprintf(dest, dest_size, "%.3f", minutes);

    char *dot = strchr(dest, '.');

    if (dot == NULL) {
        return;
    }

    char *end = dest + strlen(dest);

    while (end > dot + 1 && end[-1] == '0') {
        end--;
    }

    if (end == dot + 1) {
        end = dot;
    }

    *end = '\0';
}

static int dhcpv4_view_render_detail(int x, int y, int content_width, const char *label,
                                     const char *value)
{
    tb_print(x, y, TUI_COLOR_BLACK | TB_BOLD, TUI_COLOR_SILVER, label);

    const int value_x = x + DHCP_LABEL_WIDTH;
    const int available = content_width - DHCP_LABEL_WIDTH;

    if (value == NULL || available <= 0) {
        return y + 1;
    }

    const char *cursor = value;

    while (*cursor != '\0') {
        while (*cursor == ' ') {
            cursor++;
        }

        if (*cursor == '\0') {
            break;
        }

        const char *end = cursor;
        const char *last_space = NULL;
        int length = 0;

        while (*end != '\0' && length < available) {
            if (*end == ' ') {
                last_space = end;
            }

            end++;
            length++;
        }

        if (*end != '\0' && last_space != NULL && last_space > cursor) {
            end = last_space;
        }

        char segment[MEDIUM_BUFFER];
        size_t segment_length = (size_t)(end - cursor);

        if (segment_length >= sizeof(segment)) {
            segment_length = sizeof(segment) - 1;
        }

        memcpy(segment, cursor, segment_length);
        segment[segment_length] = '\0';

        tb_print(value_x, y, TUI_COLOR_BLACK, TUI_COLOR_SILVER, segment);

        y++;
        cursor = end;

        while (*cursor == ' ') {
            cursor++;
        }
    }

    return y;
}

void dhcpv4_view_render(const dhcpv4_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    dhcpv4_view_render_input(screen, layout);

    if (!screen->has_result) {
        return;
    }

    const int start_y = layout->content_y + 2;

    if (!screen->is_valid) {
        ui_render_input_error(layout->content_x, start_y, screen->status.message);
        return;
    }

    char t0_timer[SMALL_BUFFER];
    char t1_timer[SMALL_BUFFER];
    char t2_timer[SMALL_BUFFER];
    char t0_duration[MINI_BUFFER];
    char t1_duration[MINI_BUFFER];
    char t2_duration[MINI_BUFFER];

    dhcpv4_view_format_duration(screen->result.t0_minutes, t0_duration, sizeof(t0_duration));
    dhcpv4_view_format_duration(screen->result.t1_minutes, t1_duration, sizeof(t1_duration));
    dhcpv4_view_format_duration(screen->result.t2_minutes, t2_duration, sizeof(t2_duration));

    snprintf(t0_timer, sizeof(t0_timer), "%s minutes (%.1f%%)", t0_duration,
             (screen->result.t0_minutes / (double)screen->result.lease_minutes) * 100.0);
    snprintf(t1_timer, sizeof(t1_timer), "%s minutes (%.1f%%)", t1_duration,
             (screen->result.t1_minutes / (double)screen->result.lease_minutes) * 100.0);
    snprintf(t2_timer, sizeof(t2_timer), "%s minutes (%.1f%%)", t2_duration,
             (screen->result.t2_minutes / (double)screen->result.lease_minutes) * 100.0);

    int y = start_y;

    ui_render_section_title(layout->content_x, y, "[ RENEWAL TIMERS ]");
    y++;
    y = dhcpv4_view_render_detail(layout->content_x, y, layout->content_w,
                                  "T0 (Renewal Timer):", t0_timer);
    y = dhcpv4_view_render_detail(layout->content_x, y, layout->content_w,
                                  "Status:", "Unicast DHCPREQUEST sent to original server.");
    y++;
    y = dhcpv4_view_render_detail(layout->content_x, y, layout->content_w,
                                  "T1 (Rebinding Timer):", t1_timer);
    y = dhcpv4_view_render_detail(layout->content_x, y, layout->content_w,
                                  "Status:", "Broadcast DHCPREQUEST sent to any server.");
    y++;
    y = dhcpv4_view_render_detail(layout->content_x, y, layout->content_w,
                                  "T2 (Expiration):", t2_timer);
    (void)dhcpv4_view_render_detail(layout->content_x, y, layout->content_w,
                                    "Status:", "IP address released. Client must restart DORA.");
}
