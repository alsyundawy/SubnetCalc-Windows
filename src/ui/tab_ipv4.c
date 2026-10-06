#include "tab_ipv4.h"
#include "theme.h"
#include "clipboard.h"
#include "persist/history.h"
#include "engine/ipv4.h"
#include "engine/cloud.h"
#include "engine/classify.h"
#include <commctrl.h>
#include <stdio.h>

static HWND s_hWnd = NULL;
static HWND s_hEditIP = NULL;
static HWND s_hComboMask = NULL;
static HWND s_hSliderPrefix = NULL;
static HWND s_hComboCloud = NULL;
static HWND s_hBtnCopy = NULL;

/* Result Fields */
static HWND s_hValSubnetId = NULL;
static HWND s_hValBroadcast = NULL;
static HWND s_hValNetmask = NULL;
static HWND s_hValWildcard = NULL;
static HWND s_hValRange = NULL;
static HWND s_hValHosts = NULL;
static HWND s_hValClass = NULL;
static HWND s_hValClassBits = NULL;
static HWND s_hValSubnetBits = NULL;
static HWND s_hValHostBits = NULL;
static HWND s_hValMaxSubnets = NULL;
static HWND s_hValBinary = NULL;
static HWND s_hValHex = NULL;
static HWND s_hValBitmap = NULL;
static HWND s_hValClassify = NULL;

static uint32_t s_current_ip = 0xC0A8010A; /* 192.168.1.10 */
static int s_current_prefix = 24;
static cloud_profile_id_t s_current_cloud = CLOUD_PROFILE_STANDARD;
static bool s_updating_ui = false;

static void update_calculations(void);

void tab_ipv4_get_current_state(uint32_t *out_ip, int *out_prefix) {
    if (out_ip)
        *out_ip = s_current_ip;
    if (out_prefix)
        *out_prefix = s_current_prefix;
}

static void update_calculations(void) {
    if (s_updating_ui)
        return;
    s_updating_ui = true;

    char ip_str[IPV4_STR_BUFFER_SIZE];
    ipv4_format(s_current_ip, ip_str, sizeof(ip_str));

    /* Sync text box if needed */
    wchar_t wip[64];
    GetWindowTextW(s_hEditIP, wip, 64);
    char cur_txt[64];
    snprintf(cur_txt, sizeof(cur_txt), "%ls", wip);
    uint32_t parsed = 0;
    if (!ipv4_parse(cur_txt, &parsed) || parsed != s_current_ip) {
        wchar_t buf[64];
        swprintf(buf, 64, L"%u.%u.%u.%u", (s_current_ip >> 24) & 0xFF, (s_current_ip >> 16) & 0xFF,
                 (s_current_ip >> 8) & 0xFF, s_current_ip & 0xFF);
        SetWindowTextW(s_hEditIP, buf);
    }

    /* Sync slider */
    SendMessageW(s_hSliderPrefix, TBM_SETPOS, TRUE, (LPARAM)s_current_prefix);
    SendMessageW(s_hComboMask, CB_SETCURSEL, (WPARAM)s_current_prefix, 0);

    /* Engine Calculations */
    uint32_t net = ipv4_network_address(s_current_ip, s_current_prefix);
    uint32_t bcast = ipv4_broadcast_address(s_current_ip, s_current_prefix);
    uint32_t mask = ipv4_prefix_to_mask(s_current_prefix);
    uint32_t wild = ipv4_wildcard_mask(s_current_prefix);
    char net_class = ipv4_net_class(s_current_ip);
    int cbits = ipv4_class_bits(net_class);
    int sbits = ipv4_subnet_bits(net_class, s_current_prefix);
    int hbits = ipv4_host_bits(s_current_prefix);
    uint64_t max_subs = ipv4_max_subnets(sbits);

    /* Cloud vs Standard Usable Range */
    uint32_t u_start = 0, u_end = 0, u_count = 0;
    bool cloud_ok = cloud_profile_usable_range(s_current_cloud, net, s_current_prefix, &u_start,
                                               &u_end, &u_count);

    char net_str[IPV4_STR_BUFFER_SIZE], bcast_str[IPV4_STR_BUFFER_SIZE];
    char mask_str[IPV4_STR_BUFFER_SIZE], wild_str[IPV4_STR_BUFFER_SIZE];
    char bin_str[IPV4_STR_BUFFER_SIZE], hex_str[IPV4_STR_BUFFER_SIZE];
    char bmap_str[IPV4_BITMAP_BUFFER_SIZE];

    ipv4_format(net, net_str, sizeof(net_str));
    ipv4_format(bcast, bcast_str, sizeof(bcast_str));
    ipv4_format(mask, mask_str, sizeof(mask_str));
    ipv4_format(wild, wild_str, sizeof(wild_str));
    ipv4_binarize(s_current_ip, true, bin_str, sizeof(bin_str));
    ipv4_hexarize(s_current_ip, true, hex_str, sizeof(hex_str));
    ipv4_bitmap(net_class, s_current_prefix, true, bmap_str, sizeof(bmap_str));
    const char *classify_str = classify_ipv4(s_current_ip);

    wchar_t wbuf[128];

    /* Update labels */
    swprintf(wbuf, 128, L"%hs", net_str);
    SetWindowTextW(s_hValSubnetId, wbuf);

    swprintf(wbuf, 128, L"%hs", bcast_str);
    SetWindowTextW(s_hValBroadcast, wbuf);

    swprintf(wbuf, 128, L"%hs", mask_str);
    SetWindowTextW(s_hValNetmask, wbuf);

    swprintf(wbuf, 128, L"%hs", wild_str);
    SetWindowTextW(s_hValWildcard, wbuf);

    if (cloud_ok) {
        char u_start_str[IPV4_STR_BUFFER_SIZE], u_end_str[IPV4_STR_BUFFER_SIZE];
        ipv4_format(u_start, u_start_str, sizeof(u_start_str));
        ipv4_format(u_end, u_end_str, sizeof(u_end_str));
        swprintf(wbuf, 128, L"%hs - %hs", u_start_str, u_end_str);
        SetWindowTextW(s_hValRange, wbuf);
        swprintf(wbuf, 128, L"%u", u_count);
        SetWindowTextW(s_hValHosts, wbuf);
    } else {
        SetWindowTextW(s_hValRange, L"Prohibited by Cloud Profile");
        SetWindowTextW(s_hValHosts, L"0");
    }

    swprintf(wbuf, 128, L"Class %c", net_class);
    SetWindowTextW(s_hValClass, wbuf);

    swprintf(wbuf, 128, L"%d", cbits);
    SetWindowTextW(s_hValClassBits, wbuf);

    swprintf(wbuf, 128, L"%d", sbits);
    SetWindowTextW(s_hValSubnetBits, wbuf);

    swprintf(wbuf, 128, L"%d", hbits);
    SetWindowTextW(s_hValHostBits, wbuf);

    swprintf(wbuf, 128, L"%llu", (unsigned long long)max_subs);
    SetWindowTextW(s_hValMaxSubnets, wbuf);

    swprintf(wbuf, 128, L"%hs", bin_str);
    SetWindowTextW(s_hValBinary, wbuf);

    swprintf(wbuf, 128, L"%hs", hex_str);
    SetWindowTextW(s_hValHex, wbuf);

    swprintf(wbuf, 128, L"%hs", bmap_str);
    SetWindowTextW(s_hValBitmap, wbuf);

    swprintf(wbuf, 128, L"%hs", classify_str);
    SetWindowTextW(s_hValClassify, wbuf);

    /* Record in history */
    wchar_t hist[64];
    swprintf(hist, 64, L"%u.%u.%u.%u/%d", (s_current_ip >> 24) & 0xFF, (s_current_ip >> 16) & 0xFF,
             (s_current_ip >> 8) & 0xFF, s_current_ip & 0xFF, s_current_prefix);
    history_add(hist);

    s_updating_ui = false;
}

void tab_ipv4_copy_results(HWND hwnd) {
    wchar_t full_copy[1024];
    wchar_t w_net[64], w_bcast[64], w_mask[64], w_wild[64], w_range[64], w_hosts[64];
    wchar_t w_class[64], w_bin[64], w_hex[64], w_bmap[64], w_cls[64];

    GetWindowTextW(s_hValSubnetId, w_net, 64);
    GetWindowTextW(s_hValBroadcast, w_bcast, 64);
    GetWindowTextW(s_hValNetmask, w_mask, 64);
    GetWindowTextW(s_hValWildcard, w_wild, 64);
    GetWindowTextW(s_hValRange, w_range, 64);
    GetWindowTextW(s_hValHosts, w_hosts, 64);
    GetWindowTextW(s_hValClass, w_class, 64);
    GetWindowTextW(s_hValBinary, w_bin, 64);
    GetWindowTextW(s_hValHex, w_hex, 64);
    GetWindowTextW(s_hValBitmap, w_bmap, 64);
    GetWindowTextW(s_hValClassify, w_cls, 64);

    snwprintf(full_copy, 1024,
              L"SubnetCalc IPv4 Calculation Results\r\n"
              L"==================================\r\n"
              L"IP Address:      %u.%u.%u.%u/%d\r\n"
              L"Subnet ID:       %s\r\n"
              L"Broadcast:       %s\r\n"
              L"Subnet Mask:     %s\r\n"
              L"Wildcard:        %s\r\n"
              L"Usable Range:    %s\r\n"
              L"Usable Hosts:    %s\r\n"
              L"Classification:  %s (%s)\r\n"
              L"Binary Map:      %s\r\n"
              L"Hexadecimal:     %s\r\n"
              L"Bitmap (n/s/h):  %s\r\n",
              (s_current_ip >> 24) & 0xFF, (s_current_ip >> 16) & 0xFF, (s_current_ip >> 8) & 0xFF,
              s_current_ip & 0xFF, s_current_prefix, w_net, w_bcast, w_mask, w_wild, w_range,
              w_hosts, w_cls, w_class, w_bin, w_hex, w_bmap);

    clipboard_copy_text(hwnd, full_copy);
    MessageBoxW(hwnd, L"IPv4 calculation results copied to clipboard.", L"SubnetCalc",
                MB_OK | MB_ICONINFORMATION);
}

static LRESULT CALLBACK tab_ipv4_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        if (id == 101 && code == EN_CHANGE && !s_updating_ui) {
            wchar_t text[64];
            GetWindowTextW(s_hEditIP, text, 64);
            char ctext[64];
            snprintf(ctext, sizeof(ctext), "%ls", text);
            uint32_t ip = 0;
            if (ipv4_parse(ctext, &ip)) {
                s_current_ip = ip;
                update_calculations();
            }
        } else if (id == 102 && code == CBN_SELCHANGE) {
            int sel = (int)SendMessageW(s_hComboMask, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel <= 32) {
                s_current_prefix = sel;
                update_calculations();
            }
        } else if (id == 104 && code == CBN_SELCHANGE) {
            int sel = (int)SendMessageW(s_hComboCloud, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < CLOUD_PROFILE_COUNT) {
                s_current_cloud = (cloud_profile_id_t)sel;
                update_calculations();
            }
        } else if (id == 105) {
            tab_ipv4_copy_results(hwnd);
        }
        break;
    }
    case WM_HSCROLL: {
        if ((HWND)lParam == s_hSliderPrefix && !s_updating_ui) {
            int pos = (int)SendMessageW(s_hSliderPrefix, TBM_GETPOS, 0, 0);
            if (pos >= 0 && pos <= 32) {
                s_current_prefix = pos;
                update_calculations();
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

HWND create_tab_ipv4_window(HWND hParent, HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = tab_ipv4_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SubnetCalcTabIPv4";
    wc.hbrBackground = theme_brush_bg();
    RegisterClassExW(&wc);

    s_hWnd = CreateWindowExW(0, L"SubnetCalcTabIPv4", L"", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 900,
                             520, hParent, NULL, hInstance, NULL);

    /* Inputs: IP Address, Mask, Slider, Cloud */
    CreateWindowExW(0, L"STATIC", L"IP Address:", WS_CHILD | WS_VISIBLE, 20, 20, 100, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hEditIP = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"192.168.1.10",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 120, 18, 140,
                                24, s_hWnd, (HMENU)101, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Subnet Mask:", WS_CHILD | WS_VISIBLE, 280, 20, 90, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hComboMask = CreateWindowExW(
        0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        375, 18, 160, 300, s_hWnd, (HMENU)102, hInstance, NULL);

    for (int p = 0; p <= 32; p++) {
        uint32_t m = ipv4_prefix_to_mask(p);
        wchar_t item[64];
        swprintf(item, 64, L"/%d (%u.%u.%u.%u)", p, (m >> 24) & 0xFF, (m >> 16) & 0xFF,
                 (m >> 8) & 0xFF, m & 0xFF);
        SendMessageW(s_hComboMask, CB_ADDSTRING, 0, (LPARAM)item);
    }

    CreateWindowExW(0, L"STATIC", L"Prefix Slider:", WS_CHILD | WS_VISIBLE, 20, 55, 100, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hSliderPrefix = CreateWindowExW(0, TRACKBAR_CLASSW, L"",
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_AUTOTICKS,
                                      120, 50, 415, 30, s_hWnd, (HMENU)103, hInstance, NULL);
    SendMessageW(s_hSliderPrefix, TBM_SETRANGE, TRUE, MAKELPARAM(0, 32));
    SendMessageW(s_hSliderPrefix, TBM_SETPAGESIZE, 0, 4);

    CreateWindowExW(0, L"STATIC", L"Cloud Profile:", WS_CHILD | WS_VISIBLE, 560, 20, 90, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hComboCloud =
        CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                        655, 18, 180, 150, s_hWnd, (HMENU)104, hInstance, NULL);
    SendMessageW(s_hComboCloud, CB_ADDSTRING, 0, (LPARAM)L"Standard (RFC 1918)");
    SendMessageW(s_hComboCloud, CB_ADDSTRING, 0, (LPARAM)L"AWS VPC");
    SendMessageW(s_hComboCloud, CB_ADDSTRING, 0, (LPARAM)L"Azure VNet");
    SendMessageW(s_hComboCloud, CB_ADDSTRING, 0, (LPARAM)L"Google Cloud (GCP)");
    SendMessageW(s_hComboCloud, CB_ADDSTRING, 0, (LPARAM)L"Oracle Cloud (OCI)");
    SendMessageW(s_hComboCloud, CB_SETCURSEL, 0, 0);

    s_hBtnCopy = CreateWindowExW(0, L"BUTTON", L"Copy Results (Ctrl+C)",
                                 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 655, 50, 180,
                                 28, s_hWnd, (HMENU)105, hInstance, NULL);

    /* Result Labels Grid */
    int y = 95;
    int col1_x = 20, col1_val = 140;
    int col2_x = 450, col2_val = 570;

#define CREATE_ROW(l1, h1, l2, h2)                                                                 \
    CreateWindowExW(0, L"STATIC", l1, WS_CHILD | WS_VISIBLE, col1_x, y, 115, 20, s_hWnd, NULL,     \
                    hInstance, NULL);                                                              \
    h1 = CreateWindowExW(WS_EX_STATICEDGE, L"STATIC", L"-", WS_CHILD | WS_VISIBLE, col1_val, y,    \
                         280, 20, s_hWnd, NULL, hInstance, NULL);                                  \
    CreateWindowExW(0, L"STATIC", l2, WS_CHILD | WS_VISIBLE, col2_x, y, 115, 20, s_hWnd, NULL,     \
                    hInstance, NULL);                                                              \
    h2 = CreateWindowExW(WS_EX_STATICEDGE, L"STATIC", L"-", WS_CHILD | WS_VISIBLE, col2_val, y,    \
                         280, 20, s_hWnd, NULL, hInstance, NULL);                                  \
    y += 26;

    CREATE_ROW(L"Subnet ID:", s_hValSubnetId, L"Network Class:", s_hValClass)
    CREATE_ROW(L"Broadcast:", s_hValBroadcast, L"Class Bits:", s_hValClassBits)
    CREATE_ROW(L"Subnet Mask:", s_hValNetmask, L"Subnet Bits:", s_hValSubnetBits)
    CREATE_ROW(L"Wildcard Mask:", s_hValWildcard, L"Host Bits:", s_hValHostBits)
    CREATE_ROW(L"Usable Range:", s_hValRange, L"Max Subnets:", s_hValMaxSubnets)
    CREATE_ROW(L"Usable Hosts:", s_hValHosts, L"Classification:", s_hValClassify)

    y += 10;
    CreateWindowExW(0, L"STATIC", L"Binary Map:", WS_CHILD | WS_VISIBLE, col1_x, y, 115, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hValBinary = CreateWindowExW(WS_EX_STATICEDGE, L"STATIC", L"-", WS_CHILD | WS_VISIBLE,
                                   col1_val, y, 710, 20, s_hWnd, NULL, hInstance, NULL);
    y += 26;

    CreateWindowExW(0, L"STATIC", L"Hexadecimal:", WS_CHILD | WS_VISIBLE, col1_x, y, 115, 20,
                    s_hWnd, NULL, hInstance, NULL);
    s_hValHex = CreateWindowExW(WS_EX_STATICEDGE, L"STATIC", L"-", WS_CHILD | WS_VISIBLE, col1_val,
                                y, 710, 20, s_hWnd, NULL, hInstance, NULL);
    y += 26;

    CreateWindowExW(0, L"STATIC", L"Bitmap (n/s/h):", WS_CHILD | WS_VISIBLE, col1_x, y, 115, 20,
                    s_hWnd, NULL, hInstance, NULL);
    s_hValBitmap = CreateWindowExW(WS_EX_STATICEDGE, L"STATIC", L"-", WS_CHILD | WS_VISIBLE,
                                   col1_val, y, 710, 20, s_hWnd, NULL, hInstance, NULL);

    update_calculations();
    return s_hWnd;
}
