#ifndef SUBNETCALC_UI_TAB_HOSTS_H
#define SUBNETCALC_UI_TAB_HOSTS_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <stdint.h>

HWND create_tab_hosts_window(HWND hParent, HINSTANCE hInstance);
void tab_hosts_update(uint32_t ip, int prefix);
void tab_hosts_export_csv(HWND hwnd);
void tab_hosts_export_ascii(HWND hwnd);

#endif /* SUBNETCALC_UI_TAB_HOSTS_H */
