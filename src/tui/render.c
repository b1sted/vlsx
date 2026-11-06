/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/render.h"

#include <stddef.h>
#include <string.h>

#include "termbox2.h"

#include "app/app.h"
#include "screens/ip/ip_screen.h"
#include "tui/backend.h"
#include "tui/chrome.h"
#include "tui/layout.h"
#include "tui/theme.h"
#include "tui/views/ip/ip_tabbar.h"
#include "tui/views/ip/ipv4_view.h"
#include "tui/views/ip/ipv6_view.h"
#include "tui/views/ip/vlsm_view.h"
#include "tui/views/l2/mac_converter_view.h"
#include "tui/views/l2/mac_multicast_view.h"
#include "tui/views/l2/mac_tabbar.h"

static void render_content(const layout_t *layout);
static void render_too_small(int width, int height);

static void render_content(const layout_t *layout)
{
    switch (app.current_screen) {
    case APP_SCR_MAIN:
        break;
    case APP_SCR_IP:
        switch (ip_screen_active_tab(&app.ip_screen)) {
        case IP_TAB_V4:
            ipv4_view_render(&app.ip_screen.ipv4, layout);
            break;
        case IP_TAB_VLSM:
            vlsm_view_render(&app.ip_screen.vlsm, layout);
            break;
        case IP_TAB_V6:
            ipv6_view_render(&app.ip_screen.ipv6, layout);
            break;
        case IP_TAB_COUNT:
            break;
        }
        break;
    case APP_SCR_MAC:
        switch (mac_screen_active_tab(&app.mac_screen)) {
        case MAC_TAB_CONVERTER:
            mac_converter_view_render(&app.mac_screen.converter, layout);
            break;
        case MAC_TAB_MULTICAST:
            mac_multicast_view_render(&app.mac_screen.multicast, layout);
            break;
        case MAC_TAB_COUNT:
            break;
        }
        break;
    }
}

static void render_too_small(int width, int height)
{
    const char *size_err = "Terminal is too small!";
    const char *size_info = "Minimum required terminal size: 90x24";

    const int size_err_len = (int)strlen(size_err);
    const int size_info_len = (int)strlen(size_info);

    tb_print((width - size_err_len) / 2, height / 2 - 1, TUI_COLOR_WHITE | TB_BOLD, TB_DEFAULT,
             size_err);
    tb_print((width - size_info_len) / 2, height / 2, TUI_COLOR_WHITE | TB_BOLD, TB_DEFAULT,
             size_info);
}

void render_frame(void)
{
    tui_backend_clear();

    const int width = tui_backend_width();
    const int height = tui_backend_height();

    if (width < TUI_MIN_WIDTH || height < TUI_MIN_HEIGHT) {
        render_too_small(width, height);
        tui_backend_present();
        return;
    }

    layout_t layout = {0};

    layout_calculate(width, height, &layout);

    chrome_draw_background(width, height);
    chrome_draw_window(&layout);
    chrome_draw_titlebar(&layout);
    chrome_draw_main_menu(&app, &layout);
    chrome_draw_divider(&layout);

    if (app.current_screen == APP_SCR_IP) {
        ip_tabbar_render(&app, &layout);
    } else if (app.current_screen == APP_SCR_MAC) {
        mac_tabbar_render(&app, &layout);
    }

    render_content(&layout);

    tui_backend_present();
}
