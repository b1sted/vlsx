/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef SCREEN_IP
#define SCREEN_IP

void screen_ip_focus(void);
void screen_ip_confirm(void);

void render_ip_tabbar(const int start_x, const int tabbar_y,
                      const char *tabbar_menu[]);
void render_ip_content(const int start_x, const int start_y);

#endif /* SCREEN_IP */
