#ifndef SUBNETCALC_UI_TABS_H
#define SUBNETCALC_UI_TABS_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <commctrl.h>

typedef enum {
    TAB_IPV4 = 0,
    TAB_HOSTS,
    TAB_CIDR,
    TAB_FLSM,
    TAB_VLSM,
    TAB_IPV6,
    TAB_COUNT
} tab_index_t;

HWND create_main_tabs(HWND hParent, HINSTANCE hInstance);
void resize_main_tabs(HWND hTabCtrl, int x, int y, int width, int height);
void select_tab(HWND hTabCtrl, tab_index_t tab);
tab_index_t get_current_tab(HWND hTabCtrl);
HWND get_tab_window(tab_index_t tab);

#endif /* SUBNETCALC_UI_TABS_H */
