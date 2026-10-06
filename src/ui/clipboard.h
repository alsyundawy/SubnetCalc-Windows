#ifndef SUBNETCALC_UI_CLIPBOARD_H
#define SUBNETCALC_UI_CLIPBOARD_H

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <stdbool.h>

bool clipboard_copy_text(HWND hwnd, const wchar_t *text);

#endif /* SUBNETCALC_UI_CLIPBOARD_H */
