/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TUI_THEME_H
#define VLSX_TUI_THEME_H

#define TUI_ARRAY_LEN(array) (sizeof(array) / sizeof((array)[0]))

#define TUI_MIN_WIDTH 90
#define TUI_MIN_HEIGHT 24

#define TUI_INPUT_OFFSET 10
#define TUI_LABEL_VALUE_GAP 1

#define TUI_ARROW_LEFT 0x2192 /* -> */

#define TUI_BOX_TL 0x250C /* ┌ */
#define TUI_BOX_BL 0x2514 /* └ */

#define TUI_BOX_TR 0x2510 /* ┐ */
#define TUI_BOX_BR 0x2518 /* ┘ */

#define TUI_BOX_H 0x2500 /* ─ */
#define TUI_BOX_V 0x2502 /* │ */

#define TUI_BOX_VL 0x2524 /* ┤ */
#define TUI_BOX_VR 0x251C /* ├ */

#define TUI_BOX_BB 0x252C /* ┬ */
#define TUI_BOX_UB 0x253C /* ┼ */
#define TUI_BOX_BU 0x2534 /* ┴ */

#define TUI_COLOR_COBALT 25
#define TUI_COLOR_GREEN 82
#define TUI_COLOR_RED 196
#define TUI_COLOR_WHITE 231
#define TUI_COLOR_BLACK 232
#define TUI_COLOR_CHARCOAL 236
#define TUI_COLOR_SILVER 248

#endif /* VLSX_TUI_THEME_H */
