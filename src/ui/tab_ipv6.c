#include "tab_ipv6.h"
#include "theme.h"
#include "clipboard.h"
#include "engine/ipv6.h"
#include "engine/classify.h"
#include "engine/ula.h"
#include <commctrl.h>
#include <stdio.h>

static HWND s_hWnd = NULL;
static HWND s_hEditAddr = NULL;
static HWND s_hSliderPrefix = NULL;
static HWND s_hEditPrefix = NULL;
static HWND s_hBtnULA = NULL;
static HWND s_hBtnCopy = NULL;

static HWND s_hValCompact = NULL;
static HWND s_hValExpanded = NULL;
static HWND s_hValNetwork = NULL;
static HWND s_hValRange = NULL;
static HWND s_hValHosts = NULL;
static HWND s_hValMapped = NULL;
static HWND s_hVal6to4 = NULL;
static HWND s_hValARPA = NULL;
static HWND s_hValClassify = NULL;

static ipv6_addr_t s_current_addr;
static int s_current_prefix = 64;
static bool s_updating_ui = false;

static void update_ipv6_calculations(void);

static void update_ipv6_calculations(void) {
    if (s_updating_ui)
        return;
    s_updating_ui = true;

    char compact[IPV6_COMPACT_BUFFER_SIZE];
    char expanded[IPV6_EXPANDED_BUFFER_SIZE];
    char net_str[IPV6_COMPACT_BUFFER_SIZE];
    char s_str[IPV6_COMPACT_BUFFER_SIZE], e_str[IPV6_COMPACT_BUFFER_SIZE];
    char arpa[IPV6_ARPA_BUFFER_SIZE];
    char hosts[64];

    ipv6_format_compact(&s_current_addr, compact, sizeof(compact));
    ipv6_format_expanded(&s_current_addr, expanded, sizeof(expanded));

    ipv6_addr_t net, end;
    ipv6_range(&s_current_addr, s_current_prefix, &net, &end);
    ipv6_format_compact(&net, net_str, sizeof(net_str));
    ipv6_format_compact(&net, s_str, sizeof(s_str));
    ipv6_format_compact(&end, e_str, sizeof(e_str));

    ipv6_arpa(&s_current_addr, arpa, sizeof(arpa));
    ipv6_total_hosts(s_current_prefix, hosts, sizeof(hosts));
    const char *classify = classify_ipv6(s_current_addr.w);

    uint32_t ip4 = 0;
    char mapped_str[64] = "N/A";
    if (ipv6_is_ipv4_mapped(&s_current_addr, &ip4)) {
        snprintf(mapped_str, sizeof(mapped_str), "%u.%u.%u.%u", (ip4 >> 24) & 0xFF,
                 (ip4 >> 16) & 0xFF, (ip4 >> 8) & 0xFF, ip4 & 0xFF);
    }
    char s6to4_str[64] = "N/A";
    if (ipv6_is_6to4(&s_current_addr, &ip4)) {
        snprintf(s6to4_str, sizeof(s6to4_str), "%u.%u.%u.%u", (ip4 >> 24) & 0xFF,
                 (ip4 >> 16) & 0xFF, (ip4 >> 8) & 0xFF, ip4 & 0xFF);
    }

    wchar_t wbuf[256];

    swprintf(wbuf, 256, L"%hs", compact);
    SetWindowTextW(s_hValCompact, wbuf);

    swprintf(wbuf, 256, L"%hs", expanded);
    SetWindowTextW(s_hValExpanded, wbuf);

    swprintf(wbuf, 256, L"%hs/%d", net_str, s_current_prefix);
    SetWindowTextW(s_hValNetwork, wbuf);

    swprintf(wbuf, 256, L"%hs - %hs", s_str, e_str);
    SetWindowTextW(s_hValRange, wbuf);

    swprintf(wbuf, 256, L"%hs", hosts);
    SetWindowTextW(s_hValHosts, wbuf);

    swprintf(wbuf, 256, L"%hs", arpa);
    SetWindowTextW(s_hValARPA, wbuf);

    swprintf(wbuf, 256, L"%hs", classify);
    SetWindowTextW(s_hValClassify, wbuf);

    swprintf(wbuf, 256, L"%hs", mapped_str);
    SetWindowTextW(s_hValMapped, wbuf);

    swprintf(wbuf, 256, L"%hs", s6to4_str);
    SetWindowTextW(s_hVal6to4, wbuf);

    /* Sync controls */
    swprintf(wbuf, 256, L"%d", s_current_prefix);
    SetWindowTextW(s_hEditPrefix, wbuf);
    SendMessageW(s_hSliderPrefix, TBM_SETPOS, TRUE, (LPARAM)s_current_prefix);

    s_updating_ui = false;
}

void tab_ipv6_copy_results(HWND hwnd) {
    wchar_t w_comp[64], w_exp[64], w_net[64], w_range[128], w_hosts[64], w_arpa[128], w_cls[64];
    GetWindowTextW(s_hValCompact, w_comp, 64);
    GetWindowTextW(s_hValExpanded, w_exp, 64);
    GetWindowTextW(s_hValNetwork, w_net, 64);
    GetWindowTextW(s_hValRange, w_range, 128);
    GetWindowTextW(s_hValHosts, w_hosts, 64);
    GetWindowTextW(s_hValARPA, w_arpa, 128);
    GetWindowTextW(s_hValClassify, w_cls, 64);

    wchar_t full[1024];
    snwprintf(full, 1024,
              L"SubnetCalc IPv6 Calculation Results\r\n"
              L"==================================\r\n"
              L"Compact Address: %s\r\n"
              L"Expanded Address:%s\r\n"
              L"Network / Prefix:%s\r\n"
              L"Network Range:   %s\r\n"
              L"Total Hosts:     %s\r\n"
              L"Classification:  %s\r\n"
              L"ip6.arpa PTR:    %s\r\n",
              w_comp, w_exp, w_net, w_range, w_hosts, w_cls, w_arpa);

    clipboard_copy_text(hwnd, full);
    MessageBoxW(hwnd, L"IPv6 calculation results copied to clipboard.", L"SubnetCalc",
                MB_OK | MB_ICONINFORMATION);
}

static LRESULT CALLBACK tab_ipv6_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        if (id == 601 && code == EN_CHANGE && !s_updating_ui) {
            wchar_t wtxt[128];
            GetWindowTextW(s_hEditAddr, wtxt, 128);
            char ctxt[128];
            snprintf(ctxt, sizeof(ctxt), "%ls", wtxt);
            ipv6_addr_t parsed;
            if (ipv6_parse(ctxt, &parsed)) {
                s_current_addr = parsed;
                update_ipv6_calculations();
            }
        } else if (id == 602 && code == EN_CHANGE && !s_updating_ui) {
            wchar_t wpref[16];
            GetWindowTextW(s_hEditPrefix, wpref, 16);
            int pref = _wtoi(wpref);
            if (pref >= 0 && pref <= 128) {
                s_current_prefix = pref;
                update_ipv6_calculations();
            }
        } else if (id == 604) { /* Generate ULA */
            char p48[64], s64[64];
            ipv6_addr_t a48, a64;
            if (ula_generate(&a48, &a64, p48, sizeof(p48), s64, sizeof(s64))) {
                s_current_addr = a64;
                s_current_prefix = 64;
                wchar_t wula[64];
                swprintf(wula, 64, L"%hs", s64);
                SetWindowTextW(s_hEditAddr, wula);
                update_ipv6_calculations();
            }
        } else if (id == 605) { /* Copy */
            tab_ipv6_copy_results(hwnd);
        }
        break;
    }
    case WM_HSCROLL: {
        if ((HWND)lParam == s_hSliderPrefix && !s_updating_ui) {
            int pos = (int)SendMessageW(s_hSliderPrefix, TBM_GETPOS, 0, 0);
            if (pos >= 0 && pos <= 128) {
                s_current_prefix = pos;
                update_ipv6_calculations();
            }
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, theme_color_fg());
        SetBkColor(hdcStatic, theme_color_bg());
        return (INT_PTR)theme_brush_bg();
    }
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

HWND create_tab_ipv6_window(HWND hParent, HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = tab_ipv6_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SubnetCalcTabIPv6";
    wc.hbrBackground = theme_brush_bg();
    RegisterClassExW(&wc);

    s_hWnd = CreateWindowExW(0, L"SubnetCalcTabIPv6", L"", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 900,
                             520, hParent, NULL, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"IPv6 Address:", WS_CHILD | WS_VISIBLE, 20, 18, 100, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hEditAddr = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"2001:db8::1",
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 120, 16, 320,
                                  24, s_hWnd, (HMENU)601, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Prefix:", WS_CHILD | WS_VISIBLE, 455, 18, 45, 20, s_hWnd, NULL,
                    hInstance, NULL);
    s_hEditPrefix = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"64",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 505, 16, 45, 24,
                                    s_hWnd, (HMENU)602, hInstance, NULL);

    s_hBtnULA = CreateWindowExW(0, L"BUTTON", L"Generate RFC 4193 ULA",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 565, 15, 175,
                                26, s_hWnd, (HMENU)604, hInstance, NULL);
    s_hBtnCopy = CreateWindowExW(0, L"BUTTON", L"Copy Results",
                                 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 750, 15, 120,
                                 26, s_hWnd, (HMENU)605, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Prefix Slider (0..128):", WS_CHILD | WS_VISIBLE, 20, 52, 140,
                    20, s_hWnd, NULL, hInstance, NULL);
    s_hSliderPrefix = CreateWindowExW(0, TRACKBAR_CLASSW, L"",
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_AUTOTICKS,
                                      165, 48, 705, 30, s_hWnd, NULL, hInstance, NULL);
    SendMessageW(s_hSliderPrefix, TBM_SETRANGE, TRUE, MAKELPARAM(0, 128));
    SendMessageW(s_hSliderPrefix, TBM_SETPAGESIZE, 0, 16);

    int y = 90;
    int col_lbl = 20, col_val = 150, val_w = 720;

#define CREATE_V6_ROW(lbl, hwnd_val)                                                               \
    CreateWindowExW(0, L"STATIC", lbl, WS_CHILD | WS_VISIBLE, col_lbl, y, 125, 20, s_hWnd, NULL,   \
                    hInstance, NULL);                                                              \
    hwnd_val = CreateWindowExW(WS_EX_STATICEDGE, L"STATIC", L"-", WS_CHILD | WS_VISIBLE, col_val,  \
                               y, val_w, 20, s_hWnd, NULL, hInstance, NULL);                       \
    y += 28;

    CREATE_V6_ROW(L"Compact (RFC 5952):", s_hValCompact)
    CREATE_V6_ROW(L"Expanded Form:", s_hValExpanded)
    CREATE_V6_ROW(L"Network / Prefix:", s_hValNetwork)
    CREATE_V6_ROW(L"Network Range:", s_hValRange)
    CREATE_V6_ROW(L"Total Addresses:", s_hValHosts)
    CREATE_V6_ROW(L"Classification:", s_hValClassify)
    CREATE_V6_ROW(L"IPv4-Mapped:", s_hValMapped)
    CREATE_V6_ROW(L"6to4 Prefix:", s_hVal6to4)
    CREATE_V6_ROW(L"ip6.arpa PTR:", s_hValARPA)

    ipv6_parse("2001:db8::1", &s_current_addr);
    update_ipv6_calculations();
    return s_hWnd;
}
