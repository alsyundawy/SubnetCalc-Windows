#include "window.h"
#include "tabs.h"
#include "theme.h"
#include "about.h"
#include "tab_ipv4.h"
#include "tab_hosts.h"
#include "tab_flsm.h"
#include "tab_vlsm.h"
#include "tab_ipv6.h"
#include "persist/history.h"
#include <commctrl.h>
#include <stdio.h>

static HWND s_hMainWnd = NULL;
static HWND s_hTabCtrl = NULL;
static HWND s_hStatusBar = NULL;

static HMENU create_app_menu(void) {
    HMENU hMenu = CreateMenu();

    /* File Menu */
    HMENU hMenuFile = CreatePopupMenu();
    AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_EXPORT_CSV, L"Export to &CSV\tCtrl+E");
    AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_EXPORT_ASCII, L"Export to &ASCII Table");
    AppendMenuW(hMenuFile, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_EXIT, L"E&xit\tAlt+F4");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuFile, L"&File");

    /* Edit Menu */
    HMENU hMenuEdit = CreatePopupMenu();
    AppendMenuW(hMenuEdit, MF_STRING, IDM_EDIT_COPY, L"&Copy Calculation Results\tCtrl+C");
    AppendMenuW(hMenuEdit, MF_STRING, IDM_EDIT_CLEAR_HIST, L"Clear Calculation &History");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuEdit, L"&Edit");

    /* View Menu (Themes) */
    HMENU hMenuView = CreatePopupMenu();
    for (int i = 0; i < THEME_COUNT; i++) {
        AppendMenuW(hMenuView, MF_STRING, IDM_VIEW_THEME_BASE + i, theme_get_name((theme_id_t)i));
    }
    CheckMenuItem(hMenuView, IDM_VIEW_THEME_BASE + theme_get(), MF_CHECKED);
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuView, L"&View");

    /* Help Menu */
    HMENU hMenuHelp = CreatePopupMenu();
    AppendMenuW(hMenuHelp, MF_STRING, IDM_HELP_RFC, L"&RFC Notes & Algorithmic Parity");
    AppendMenuW(hMenuHelp, MF_STRING, IDM_HELP_ABOUT, L"&About SubnetCalc");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuHelp, L"&Help");

    return hMenu;
}

void update_status_bar(const wchar_t *text) {
    if (s_hStatusBar && text) {
        SendMessageW(s_hStatusBar, SB_SETTEXTW, 0, (LPARAM)text);
    }
}

static LRESULT CALLBACK main_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        theme_init();
        history_init();
        s_hTabCtrl = create_main_tabs(hwnd, ((LPCREATESTRUCTW)lParam)->hInstance);

        s_hStatusBar =
            CreateWindowExW(0, STATUSCLASSNAMEW, L"", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0,
                            0, 0, hwnd, NULL, ((LPCREATESTRUCTW)lParam)->hInstance, NULL);
        update_status_bar(
            L"Ready. SubnetCalc Windows 7 SP1+ Native C11 (Parity baseline: macOS v2.6.2)");
        return 0;
    }
    case WM_SIZE: {
        int w = LOWORD(lParam);
        int h = HIWORD(lParam);
        SendMessageW(s_hStatusBar, WM_SIZE, wParam, lParam);

        RECT rcStatus;
        GetWindowRect(s_hStatusBar, &rcStatus);
        int status_h = rcStatus.bottom - rcStatus.top;

        resize_main_tabs(s_hTabCtrl, 10, 10, w - 20, h - status_h - 20);
        return 0;
    }
    case WM_NOTIFY: {
        LPNMHDR pnm = (LPNMHDR)lParam;
        if (pnm->hwndFrom == s_hTabCtrl && pnm->code == TCN_SELCHANGE) {
            tab_index_t cur = get_current_tab(s_hTabCtrl);
            select_tab(s_hTabCtrl, cur);
        }
        break;
    }
    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        if (id == IDM_FILE_EXIT) {
            PostQuitMessage(0);
        } else if (id == IDM_FILE_EXPORT_CSV) {
            tab_index_t cur = get_current_tab(s_hTabCtrl);
            if (cur == TAB_HOSTS)
                tab_hosts_export_csv(hwnd);
            else if (cur == TAB_FLSM)
                tab_flsm_export_csv(hwnd);
            else if (cur == TAB_VLSM)
                tab_vlsm_export_csv(hwnd);
            else
                tab_hosts_export_csv(hwnd);
        } else if (id == IDM_FILE_EXPORT_ASCII) {
            tab_hosts_export_ascii(hwnd);
        } else if (id == IDM_EDIT_COPY) {
            tab_index_t cur = get_current_tab(s_hTabCtrl);
            if (cur == TAB_IPV6) {
                tab_ipv6_copy_results(hwnd);
            } else {
                tab_ipv4_copy_results(hwnd);
            }
        } else if (id == IDM_EDIT_CLEAR_HIST) {
            history_clear();
            MessageBoxW(hwnd, L"Calculation history cleared.", L"SubnetCalc",
                        MB_OK | MB_ICONINFORMATION);
        } else if (id >= IDM_VIEW_THEME_BASE && id < IDM_VIEW_THEME_BASE + THEME_COUNT) {
            theme_id_t tid = (theme_id_t)(id - IDM_VIEW_THEME_BASE);
            theme_set(tid);
            HMENU hMenu = GetMenu(hwnd);
            HMENU hView = GetSubMenu(hMenu, 2);
            for (int i = 0; i < THEME_COUNT; i++) {
                CheckMenuItem(hView, IDM_VIEW_THEME_BASE + i,
                              (i == tid) ? MF_CHECKED : MF_UNCHECKED);
            }
            InvalidateRect(hwnd, NULL, TRUE);
        } else if (id == IDM_HELP_ABOUT) {
            show_about_dialog(hwnd);
        } else if (id == IDM_HELP_RFC) {
            MessageBoxW(
                hwnd,
                L"Algorithmic Citations & Invariants:\n\n"
                L"• RFC 3021: /31 prefixes have 2 usable addresses for point-to-point links.\n"
                L"• IPv4 /32: Treated as a single host route (1 host address, 0 host bits).\n"
                L"• RFC 4193: IPv6 ULA generated via BCryptGenRandom (40 random Global ID bits).\n"
                L"• RFC 5952: IPv6 text compression (lowercase hex, longest-zero reduction).\n"
                L"• Multi-Cloud VPC Profiles: AWS (5), Azure (5), GCP (4), OCI (3), Standard (2).\n"
                L"• CWE-1236: CSV formula injection defense enabled.",
                L"RFC Notes — SubnetCalc", MB_OK | MB_ICONINFORMATION);
        }
        break;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORDLG: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, theme_color_fg());
        SetBkColor(hdc, theme_color_bg());
        return (INT_PTR)theme_brush_bg();
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

HWND create_main_window(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = main_window_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SubnetCalcMainWindow";
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(1));
    wc.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(1));
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = theme_brush_bg();
    RegisterClassExW(&wc);

    HMENU hMenu = create_app_menu();

    s_hMainWnd = CreateWindowExW(
        0, L"SubnetCalcMainWindow", L"SubnetCalc — IPv4 & IPv6 Subnet Calculator (Windows 7 SP1+)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 980, 680, NULL, hMenu, hInstance, NULL);

    if (s_hMainWnd) {
        ShowWindow(s_hMainWnd, nCmdShow);
        UpdateWindow(s_hMainWnd);
    }

    return s_hMainWnd;
}
