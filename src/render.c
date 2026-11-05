/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "termbox2.h"

#include "app.h"
#include "render.h"
#include "ui.h"

#include "screens/ip/screen_ip.h"

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define MIN_WIDTH 90
#define MIN_HEIGHT 24

typedef struct {
    const char **items;
    int count;
} tabbar_menu_t;

static void filing_background(const int width, const int height);

static void render_window(const int width, const int height);
static void render_outline(const int win_right, const int win_bottom);
static void render_titlebar(void);
static void render_main_menu(void);
static void render_divider(const int win_bottom);

static void render_tabbar(void);
static void render_content(void);

static const char *main_menu[] = {"1. IP & VLSM", "2. MAC Tools", "3. Metrics",
                                  "4. DHCP Timers"};

static const char *ip_tabs[] = {"IPv4", "VLSM", "IPv6"};
static const char *mac_tabs[] = {"Multicast MAC", "Converter"};
static const char *met_tabs[] = {"Availability", "Goodput", "Costs"};
static const char *dhcp_tabs[] = {"DHCPv4"};

static const tabbar_menu_t tabbar_menu[] = {
    {ip_tabs, ARRAY_LEN(ip_tabs)},
    {mac_tabs, ARRAY_LEN(mac_tabs)},
    {met_tabs, ARRAY_LEN(met_tabs)},
    {dhcp_tabs, ARRAY_LEN(dhcp_tabs)},
};

static int sidebar_len = 0;

static int table_text_padding(const int field_width, const int text_len,
                              const table_align_t align) {
    int padding = field_width - text_len;

    if (padding <= 0)
        return 0;

    switch (align) {
    case TABLE_ALIGN_RIGHT:
        return padding;
    case TABLE_ALIGN_CENTER:
        return padding / 2;
    case TABLE_ALIGN_LEFT:
    default:
        return 0;
    }
}

static int table_column_width(const table_t *table, const int column_index) {
    const table_column_t *column = &table->columns[column_index];
    int width = (column->width > 0) ? column->width : 0;

    if (column->title != NULL) {
        const int title_len = (int)strlen(column->title);

        if (title_len > width)
            width = title_len;
    }

    if (table->cells == NULL)
        return width;

    for (int row = 0; row < table->row_count; row++) {
        const char *cell =
            table->cells[(size_t)row * (size_t)table->column_count +
                         column_index];

        if (cell == NULL)
            continue;

        const int cell_len = (int)strlen(cell);

        if (cell_len > width)
            width = cell_len;
    }

    return width;
}

static void table_draw_char(const int x, const int y, const int width,
                            const int height, const int row_y,
                            const int logical_x, const int column_offset,
                            const uint32_t ch, const uintattr_t fg,
                            const uintattr_t bg) {
    if (row_y < y || row_y >= y + height)
        return;

    int draw_x = x + logical_x - column_offset;

    if (draw_x < x || draw_x >= x + width)
        return;

    tb_set_cell(draw_x, row_y, ch, fg, bg);
}

static void table_draw_field(const int x, const int y, const int width,
                             const int height, const int row_y,
                             const int logical_x, const int column_offset,
                             const char *text, const int field_width,
                             const table_align_t align, const uintattr_t fg,
                             const uintattr_t bg) {
    if (field_width <= 0)
        return;

    if (text == NULL)
        text = "";

    const int text_len = (int)strlen(text);
    const int padding = table_text_padding(field_width, text_len, align);

    for (int i = 0; i < field_width; i++) {
        const int text_index = i - padding;
        char ch = (text_index >= 0 && text_index < text_len)
                      ? text[text_index]
                      : ' ';

        table_draw_char(x, y, width, height, row_y, logical_x + i,
                        column_offset, (uint32_t)(unsigned char)ch, fg, bg);
    }
}

static void table_draw_row(const int x, const int y, const int width,
                           const int height, const int row_y,
                           const int column_offset, const table_t *table,
                           const char *const *cells, const bool header,
                           const uintattr_t fg, const uintattr_t bg) {
    int logical_x = 0;

    for (int i = 0; i < table->column_count; i++) {
        const table_column_t *column = &table->columns[i];
        const char *text = header ? column->title : cells[i];
        const int field_width = table_column_width(table, i);

        table_draw_field(x, y, width, height, row_y, logical_x, column_offset,
                         text, field_width, column->align, fg, bg);

        logical_x += field_width;

        if (i < table->column_count - 1) {
            table_draw_char(x, y, width, height, row_y, logical_x,
                            column_offset, '|', fg, bg);
            logical_x++;
        }
    }
}

static void table_draw_divider(const int x, const int y, const int width,
                               const int height, const int row_y,
                               const int column_offset,
                               const table_t *table) {
    const int total_width = table_total_width(table);

    for (int i = 0; i < total_width; i++) {
        table_draw_char(x, y, width, height, row_y, i, column_offset, '-',
                        COLOR_CHARCOAL, COLOR_SILVER);
    }
}

static int table_body_height(const int height) {
    return (height > 2) ? height - 2 : 0;
}

int table_total_width(const table_t *table) {
    if (table == NULL || table->columns == NULL || table->column_count <= 0)
        return 0;

    int width = table->column_count - 1;

    for (int i = 0; i < table->column_count; i++)
        width += table_column_width(table, i);

    return width;
}

int table_total_height(const table_t *table, const int height) {
    if (table == NULL || table->columns == NULL ||
        table->column_count <= 0 || height <= 0)
        return 0;

    int needed = 2; /* header + divider */

    if (table->cells != NULL && table->row_count > 0)
        needed += table->row_count;

    return (needed < height) ? needed : height;
}

int table_max_row_offset(const table_t *table, const int height) {
    if (table == NULL)
        return 0;

    const int body_height = table_body_height(height);

    if (body_height <= 0 || table->row_count <= body_height)
        return 0;

    return table->row_count - body_height;
}

int table_max_column_offset(const table_t *table, const int width) {
    const int total_width = table_total_width(table);

    if (width <= 0 || total_width <= width)
        return 0;

    return total_width - width;
}

void render_table(const int x, const int y, const int width, const int height,
                  const table_t *table, int row_offset, int column_offset) {
    if (table == NULL || table->columns == NULL || table->column_count <= 0)
        return;

    if (width <= 0 || height <= 0)
        return;

    if (row_offset < 0)
        row_offset = 0;
    else if (row_offset > table_max_row_offset(table, height))
        row_offset = table_max_row_offset(table, height);

    if (column_offset < 0)
        column_offset = 0;
    else if (column_offset > table_max_column_offset(table, width))
        column_offset = table_max_column_offset(table, width);

    table_draw_row(x, y, width, height, y, column_offset, table, NULL, true,
                   COLOR_COBALT | TB_BOLD, COLOR_SILVER);

    if (height < 2)
        return;

    table_draw_divider(x, y, width, height, y + 1, column_offset, table);

    if (table->cells == NULL || table->row_count <= 0)
        return;

    const int body_height = table_body_height(height);

    for (int i = 0; i < body_height; i++) {
        const int row = row_offset + i;

        if (row >= table->row_count)
            break;

        const char *const *cells =
            table->cells + ((size_t)row * (size_t)table->column_count);

        table_draw_row(x, y, width, height, y + 2 + i, column_offset, table,
                       cells, false, COLOR_BLACK, COLOR_SILVER);
    }
}

static void filing_background(const int width, const int height) {
    for (int row = 0; row < width; row++) {
        for (int col = 0; col < height; col++) {
            tb_set_cell(row, col, ' ', COLOR_COBALT, COLOR_COBALT);
        }
    }
}

static void render_window(const int width, const int height) {
    sizes.win_w = (width * 85) / 100;
    sizes.win_h = (height * 90) / 100;

    sizes.win_x = (width - sizes.win_w) / 2;
    sizes.win_y = (height - sizes.win_h) / 2;

    int win_right = sizes.win_x + sizes.win_w;
    int win_bottom = sizes.win_y + sizes.win_h;

    /* Draw a window background */
    for (int row = sizes.win_x; row < win_right; row++) {
        for (int col = sizes.win_y; col < win_bottom; col++) {
            tb_set_cell(row, col, ' ', COLOR_SILVER, COLOR_SILVER);
        }
    }

    /* Draw a horizontal shadow */
    for (int row = sizes.win_x + 2; row < win_right + 2; row++) {
        tb_set_cell(row, win_bottom, ' ', COLOR_CHARCOAL, COLOR_CHARCOAL);
    }

    /* Draw a vertical shadow */
    for (int col = sizes.win_y + 1; col < win_bottom; col++) {
        tb_set_cell(win_right, col, ' ', COLOR_CHARCOAL, COLOR_CHARCOAL);
        tb_set_cell(win_right + 1, col, ' ', COLOR_CHARCOAL, COLOR_CHARCOAL);
    }

    render_outline(win_right, win_bottom);
    render_titlebar();
    render_main_menu();
    render_divider(win_bottom);
}

static void render_outline(const int win_right, const int win_bottom) {
    /* Corner's */
    tb_set_cell(sizes.win_x, sizes.win_y, BOX_TL, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(win_right - 1, sizes.win_y, BOX_TR, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(sizes.win_x, win_bottom - 1, BOX_BL, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(win_right - 1, win_bottom - 1, BOX_BR, COLOR_BLACK,
                COLOR_SILVER);

    /* Vertical borders */
    for (int col = sizes.win_y + 1; col < win_bottom - 1; col++) {
        tb_set_cell(sizes.win_x, col, BOX_V, COLOR_BLACK, COLOR_SILVER);
        tb_set_cell(win_right - 1, col, BOX_V, COLOR_BLACK, COLOR_SILVER);
    }

    tb_set_cell(sizes.win_x, sizes.win_y + 2, BOX_VR, COLOR_BLACK,
                COLOR_SILVER);
    tb_set_cell(win_right - 1, sizes.win_y + 2, BOX_VL, COLOR_BLACK,
                COLOR_SILVER);

    /* Horizontal borders */
    for (int row = sizes.win_x + 1; row < win_right - 1; row++) {
        tb_set_cell(row, sizes.win_y, BOX_H, COLOR_BLACK, COLOR_SILVER);
        tb_set_cell(row, sizes.win_y + 2, BOX_H, COLOR_BLACK, COLOR_SILVER);
        tb_set_cell(row, win_bottom - 1, BOX_H, COLOR_BLACK, COLOR_SILVER);
    }
}

static void render_titlebar(void) {
    const char *title = "vlsx " APP_VERSION;
    const int title_len = (int)strlen(title);
    sidebar_len = (sidebar_len < title_len) ? title_len : sidebar_len;

    tb_print(sizes.win_x + 1, sizes.win_y + 1, COLOR_COBALT | TB_BOLD,
             COLOR_SILVER, title);
}

static void render_main_menu(void) {
    app.sidebar_items = (int)(sizeof(main_menu) / sizeof(main_menu[0]));

    int item_x = sizes.win_x + 4;
    int start_y = sizes.win_y + 3;

    for (int i = 0; i < app.sidebar_items; i++) {
        int item_len = (int)(strlen(main_menu[i]));

        bool selected = (i == app.sidebar_cursor) ? true : false;
        uintattr_t fg = selected ? COLOR_SILVER | TB_BOLD : COLOR_BLACK;
        uintattr_t bg = selected ? COLOR_COBALT : COLOR_SILVER;

        int box_x = sizes.win_x + 1;
        for (int row = box_x; row < box_x + item_len + 4; row++) {
            tb_set_cell(row, start_y, ' ', fg, bg);
        }

        if (selected) {
            int arrow_x = item_x - 2;
            tb_set_cell(arrow_x, start_y, ARROW_L, fg, bg);
        }

        tb_print(item_x, start_y, fg, bg, main_menu[i]);

        item_len += 5;
        sidebar_len = (sidebar_len < item_len) ? item_len : sidebar_len;
        start_y += 1;
    }
}

static void render_divider(const int win_bottom) {
    int divider_x = sizes.win_x + sidebar_len;

    for (int col = sizes.win_y + 1; col < win_bottom; col++) {
        tb_set_cell(divider_x, col, BOX_V, COLOR_BLACK, COLOR_SILVER);
    }

    tb_set_cell(divider_x, sizes.win_y, BOX_BB, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(divider_x, sizes.win_y + 2, BOX_UB, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(divider_x, win_bottom - 1, BOX_BU, COLOR_BLACK, COLOR_SILVER);
}

static void render_tabbar(void) {
    int start_x = sizes.win_x + sidebar_len + 3;
    int tabbar_y = sizes.win_y + 1;

    switch (app.current_screen) {
    case APP_SCR_MAIN:
        break;
    case APP_SCR_IP:
        app.tabbar_items = tabbar_menu[0].count;
        render_ip_tabbar(start_x, tabbar_y, tabbar_menu[0].items, tabbar_menu[0].count);
        break;
    }
}

static void render_content(void) {
    int start_x = sizes.win_x + sidebar_len + 2;
    int start_y = sizes.win_y + 3;

    switch (app.current_screen) {
    case APP_SCR_MAIN:
        break;
    case APP_SCR_IP:
        render_ip_content(start_x, start_y);
        break;
    }
}

void render(void) {
    tb_clear();

    int width = tb_width();
    int height = tb_height();

    if (width < MIN_WIDTH || height < MIN_HEIGHT) {
        const char *size_err = "Terminal is too small!";
        const int size_err_len = (int)strlen(size_err);

        int error_x = (width - size_err_len) / 2;
        int error_y = height / 2 - 1;
        tb_print(error_x, error_y, COLOR_WHITE | TB_BOLD, TB_DEFAULT, size_err);

        const char *size_info = "Minimum required terminal size: 90x24";
        const int size_info_len = (int)strlen(size_info);

        int info_x = (width - size_info_len) / 2;
        int info_y = height / 2;
        tb_print(info_x, info_y, COLOR_WHITE | TB_BOLD, TB_DEFAULT, size_info);

        tb_present();
        return;
    }

    filing_background(width, height);
    render_window(width, height);
    render_tabbar();
    render_content();

    tb_present();

    return;
}
