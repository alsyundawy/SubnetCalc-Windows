#ifndef SUBNETCALC_UI_TAB_FLSM_H
#define SUBNETCALC_UI_TAB_FLSM_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <stdint.h>

HWND create_tab_flsm_window(HWND hParent, HINSTANCE hInstance);
void tab_flsm_update(uint32_t ip, int prefix);
void tab_flsm_export_csv(HWND hwnd);

#endif /* SUBNETCALC_UI_TAB_FLSM_H */
