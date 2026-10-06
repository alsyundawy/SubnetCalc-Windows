#include "tab_cidr.h"
#include "theme.h"
#include "engine/ipv4.h"
#include "engine/cidr.h"
#include <stdio.h>
#include <string.h>

static HWND s_hWnd = NULL;
static HWND s_hEditInput = NULL;
static HWND s_hEditOutput = NULL;
static HWND s_hBtnAggregate = NULL;

static void do_aggregation(void) {
    wchar_t wtext[2048];
    GetWindowTextW(s_hEditInput, wtext, 2048);

    char ctext[2048];
    snprintf(ctext, sizeof(ctext), "%ls", wtext);

    cidr_block_t blocks[64];
    size_t count = 0;

    const char *line = strtok(ctext, "\r\n; ,");
    while (line && count < 64) {
        char *slash = strchr(line, '/');
        if (slash) {
            *slash = '\0';
            int pref = atoi(slash + 1);
            uint32_t ip = 0;
            if (ipv4_parse(line, &ip) && pref >= 0 && pref <= 32) {
                blocks[count].network = ip;
                blocks[count].prefix = pref;
                count++;
            }
        }
        line = strtok(NULL, "\r\n; ,");
    }

    size_t agg_count = cidr_aggregate(blocks, count);

    wchar_t out_wtext[2048] = {0};
    int pos = 0;
    for (size_t i = 0; i < agg_count; i++) {
        char net_str[32];
        ipv4_format(blocks[i].network, net_str, sizeof(net_str));
        pos += snwprintf(out_wtext + pos, 2048 - (size_t)pos, L"%hs/%d\r\n", net_str,
                         blocks[i].prefix);
    }

    SetWindowTextW(s_hEditOutput, out_wtext);
}

void tab_cidr_update(uint32_t ip, int prefix) {
    (void)ip;
    (void)prefix;
}

static LRESULT CALLBACK tab_cidr_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        if (LOWORD(wParam) == 301) {
            do_aggregation();
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

HWND create_tab_cidr_window(HWND hParent, HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = tab_cidr_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SubnetCalcTabCIDR";
    wc.hbrBackground = theme_brush_bg();
    RegisterClassExW(&wc);

    s_hWnd = CreateWindowExW(0, L"SubnetCalcTabCIDR", L"", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 900,
                             520, hParent, NULL, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Input Subnet Blocks (e.g. 192.168.0.0/24, one per line):",
                    WS_CHILD | WS_VISIBLE, 20, 15, 420, 20, s_hWnd, NULL, hInstance, NULL);
    s_hEditInput = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT",
        L"192.168.0.0/24\r\n192.168.1.0/24\r\n192.168.2.0/24\r\n192.168.3.0/24",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 20, 40,
        380, 400, s_hWnd, NULL, hInstance, NULL);

    s_hBtnAggregate = CreateWindowExW(0, L"BUTTON", L"Aggregate (Supernet) >>",
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 415, 210,
                                      175, 36, s_hWnd, (HMENU)301, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Aggregated Supernet Route(s):", WS_CHILD | WS_VISIBLE, 605, 15,
                    260, 20, s_hWnd, NULL, hInstance, NULL);
    s_hEditOutput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE |
                                        ES_READONLY | WS_VSCROLL,
                                    605, 40, 270, 400, s_hWnd, NULL, hInstance, NULL);

    return s_hWnd;
}
