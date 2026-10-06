#ifndef SUBNETCALC_UI_TAB_VLSM_H
#define SUBNETCALC_UI_TAB_VLSM_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <stdint.h>

HWND create_tab_vlsm_window(HWND hParent, HINSTANCE hInstance);
void tab_vlsm_update(uint32_t ip, int prefix);
void tab_vlsm_export_csv(HWND hwnd);

#endif /* SUBNETCALC_UI_TAB_VLSM_H */
