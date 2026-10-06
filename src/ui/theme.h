#ifndef SUBNETCALC_UI_THEME_H
#define SUBNETCALC_UI_THEME_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>

typedef enum {
    THEME_CLASSIC = 0,
    THEME_DARK,
    THEME_CATPPUCCIN,
    THEME_DRACULA,
    THEME_TOKYO_NIGHT,
    THEME_HIGH_CONTRAST,
    THEME_COUNT
} theme_id_t;

void theme_init(void);
void theme_set(theme_id_t id);
theme_id_t theme_get(void);
const wchar_t *theme_get_name(theme_id_t id);

COLORREF theme_color_bg(void);
COLORREF theme_color_fg(void);
COLORREF theme_color_surface(void);
COLORREF theme_color_accent(void);
COLORREF theme_color_badge_bg(void);
COLORREF theme_color_badge_fg(void);

HBRUSH theme_brush_bg(void);
HBRUSH theme_brush_surface(void);
HBRUSH theme_brush_badge(void);

#endif /* SUBNETCALC_UI_THEME_H */
