#include "theme.h"

typedef struct {
    COLORREF bg;
    COLORREF fg;
    COLORREF surface;
    COLORREF accent;
    COLORREF badge_bg;
    COLORREF badge_fg;
} theme_colors_t;

static theme_id_t s_current_theme = THEME_CLASSIC;

static const theme_colors_t s_palettes[THEME_COUNT] = {
    /* THEME_CLASSIC */
    {RGB(240, 240, 240), RGB(0, 0, 0), RGB(255, 255, 255), RGB(0, 102, 204), RGB(220, 230, 242),
     RGB(0, 51, 102)},
    /* THEME_DARK */
    {RGB(30, 30, 30), RGB(220, 220, 220), RGB(45, 45, 45), RGB(0, 122, 204), RGB(50, 70, 95),
     RGB(180, 215, 255)},
    /* THEME_CATPPUCCIN (Mocha) */
    {RGB(30, 30, 46), RGB(205, 214, 244), RGB(49, 50, 68), RGB(137, 180, 250), RGB(69, 71, 90),
     RGB(166, 227, 161)},
    /* THEME_DRACULA */
    {RGB(40, 42, 54), RGB(248, 248, 242), RGB(68, 71, 90), RGB(189, 147, 249), RGB(98, 114, 164),
     RGB(139, 233, 253)},
    /* THEME_TOKYO_NIGHT */
    {RGB(26, 27, 38), RGB(192, 202, 245), RGB(36, 40, 59), RGB(122, 162, 247), RGB(65, 72, 104),
     RGB(125, 207, 255)},
    /* THEME_HIGH_CONTRAST */
    {RGB(0, 0, 0), RGB(255, 255, 255), RGB(20, 20, 20), RGB(0, 255, 255), RGB(0, 0, 255),
     RGB(255, 255, 0)}};

static HBRUSH s_brush_bg = NULL;
static HBRUSH s_brush_surface = NULL;
static HBRUSH s_brush_badge = NULL;

static void update_brushes(void) {
    if (s_brush_bg)
        DeleteObject(s_brush_bg);
    if (s_brush_surface)
        DeleteObject(s_brush_surface);
    if (s_brush_badge)
        DeleteObject(s_brush_badge);

    const theme_colors_t *c = &s_palettes[s_current_theme];
    s_brush_bg = CreateSolidBrush(c->bg);
    s_brush_surface = CreateSolidBrush(c->surface);
    s_brush_badge = CreateSolidBrush(c->badge_bg);
}

void theme_init(void) {
    update_brushes();
}

void theme_set(theme_id_t id) {
    if (id < THEME_COUNT) {
        s_current_theme = id;
        update_brushes();
    }
}

theme_id_t theme_get(void) {
    return s_current_theme;
}

const wchar_t *theme_get_name(theme_id_t id) {
    switch (id) {
    case THEME_CLASSIC:
        return L"Classic";
    case THEME_DARK:
        return L"Dark";
    case THEME_CATPPUCCIN:
        return L"Catppuccin Mocha";
    case THEME_DRACULA:
        return L"Dracula";
    case THEME_TOKYO_NIGHT:
        return L"Tokyo Night";
    case THEME_HIGH_CONTRAST:
        return L"High Contrast";
    default:
        return L"Classic";
    }
}

COLORREF theme_color_bg(void) {
    return s_palettes[s_current_theme].bg;
}
COLORREF theme_color_fg(void) {
    return s_palettes[s_current_theme].fg;
}
COLORREF theme_color_surface(void) {
    return s_palettes[s_current_theme].surface;
}
COLORREF theme_color_accent(void) {
    return s_palettes[s_current_theme].accent;
}
COLORREF theme_color_badge_bg(void) {
    return s_palettes[s_current_theme].badge_bg;
}
COLORREF theme_color_badge_fg(void) {
    return s_palettes[s_current_theme].badge_fg;
}

HBRUSH theme_brush_bg(void) {
    return s_brush_bg;
}
HBRUSH theme_brush_surface(void) {
    return s_brush_surface;
}
HBRUSH theme_brush_badge(void) {
    return s_brush_badge;
}
