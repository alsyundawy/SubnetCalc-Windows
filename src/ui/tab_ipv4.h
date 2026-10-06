#ifndef SUBNETCALC_UI_TAB_IPV4_H
#define SUBNETCALC_UI_TAB_IPV4_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <stdint.h>

HWND create_tab_ipv4_window(HWND hParent, HINSTANCE hInstance);
void tab_ipv4_get_current_state(uint32_t *out_ip, int *out_prefix);
void tab_ipv4_copy_results(HWND hwnd);

#endif /* SUBNETCALC_UI_TAB_IPV4_H */
