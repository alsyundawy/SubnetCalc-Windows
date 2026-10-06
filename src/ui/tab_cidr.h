#ifndef SUBNETCALC_UI_TAB_CIDR_H
#define SUBNETCALC_UI_TAB_CIDR_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <stdint.h>

HWND create_tab_cidr_window(HWND hParent, HINSTANCE hInstance);
void tab_cidr_update(uint32_t ip, int prefix);

#endif /* SUBNETCALC_UI_TAB_CIDR_H */
