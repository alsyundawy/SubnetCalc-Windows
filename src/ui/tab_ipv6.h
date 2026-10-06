#ifndef SUBNETCALC_UI_TAB_IPV6_H
#define SUBNETCALC_UI_TAB_IPV6_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>

HWND create_tab_ipv6_window(HWND hParent, HINSTANCE hInstance);
void tab_ipv6_copy_results(HWND hwnd);

#endif /* SUBNETCALC_UI_TAB_IPV6_H */
