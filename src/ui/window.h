#ifndef SUBNETCALC_UI_WINDOW_H
#define SUBNETCALC_UI_WINDOW_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>

#define IDM_FILE_EXPORT_CSV 1001
#define IDM_FILE_EXPORT_ASCII 1002
#define IDM_FILE_EXIT 1003
#define IDM_EDIT_COPY 2001
#define IDM_EDIT_CLEAR_HIST 2002
#define IDM_VIEW_THEME_BASE 3000 /* 3000..3005 */
#define IDM_HELP_RFC 4001
#define IDM_HELP_ABOUT 4002

HWND create_main_window(HINSTANCE hInstance, int nCmdShow);
void update_status_bar(const wchar_t *text);

#endif /* SUBNETCALC_UI_WINDOW_H */
