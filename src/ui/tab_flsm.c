#include "tab_flsm.h"
#include "theme.h"
#include "engine/ipv4.h"
#include "engine/split.h"
#include "engine/export.h"
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>

static HWND s_hWnd = NULL;
static HWND s_hEditBaseIP = NULL;
static HWND s_hComboBaseMask = NULL;
static HWND s_hComboTargetMask = NULL;
static HWND s_hList = NULL;
static HWND s_hLblSummary = NULL;

static uint32_t s_base_ip = 0xC0A80100;
static int s_base_prefix = 24;
static int s_target_prefix = 26;

static void update_flsm(void) {
    SendMessageW(s_hList, LVM_DELETEALLITEMS, 0, 0);

    subnet_slice_t slices[MAX_FLSM_ROWS];
    size_t count = flsm_split(s_base_ip, s_base_prefix, s_target_prefix, slices, MAX_FLSM_ROWS);

    for (size_t i = 0; i < count; i++) {
        char net[32], bcast[32], range[64], mask[32];
        char u_start[32], u_end[32];
        ipv4_format(slices[i].network, net, sizeof(net));
        ipv4_format(slices[i].broadcast, bcast, sizeof(bcast));
        ipv4_format(slices[i].usable_start, u_start, sizeof(u_start));
        ipv4_format(slices[i].usable_end, u_end, sizeof(u_end));
        ipv4_format(slices[i].mask, mask, sizeof(mask));
        snprintf(range, sizeof(range), "%s - %s", u_start, u_end);

        wchar_t wbuf[128];
        LVITEMW lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;

        swprintf(wbuf, 128, L"%zu", i + 1);
        lvi.pszText = wbuf;
        SendMessageW(s_hList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

        swprintf(wbuf, 128, L"%hs/%d", net, slices[i].prefix);
        ListView_SetItemText(s_hList, i, 1, wbuf);

        swprintf(wbuf, 128, L"%hs", bcast);
        ListView_SetItemText(s_hList, i, 2, wbuf);

        swprintf(wbuf, 128, L"%hs", range);
        ListView_SetItemText(s_hList, i, 3, wbuf);

        swprintf(wbuf, 128, L"%hs", mask);
        ListView_SetItemText(s_hList, i, 4, wbuf);

        swprintf(wbuf, 128, L"%u", slices[i].host_count);
        ListView_SetItemText(s_hList, i, 5, wbuf);
    }

    uint32_t hosts_per = (count > 0) ? slices[0].host_count : 0;
    uint64_t total_hosts = (uint64_t)count * hosts_per;
    wchar_t sum_text[256];
    swprintf(sum_text, 256,
             L"FLSM Summary: Created %zu subnets, %u hosts per subnet (Total: %llu usable hosts)",
             count, hosts_per, (unsigned long long)total_hosts);
    SetWindowTextW(s_hLblSummary, sum_text);
}

void tab_flsm_update(uint32_t ip, int prefix) {
    s_base_ip = ip;
    s_base_prefix = prefix;
    if (s_target_prefix < s_base_prefix) {
        s_target_prefix = (s_base_prefix < 30) ? (s_base_prefix + 2) : s_base_prefix;
    }
    if (s_hList) {
        wchar_t buf[64];
        swprintf(buf, 64, L"%u.%u.%u.%u", (s_base_ip >> 24) & 0xFF, (s_base_ip >> 16) & 0xFF,
                 (s_base_ip >> 8) & 0xFF, s_base_ip & 0xFF);
        SetWindowTextW(s_hEditBaseIP, buf);
        SendMessageW(s_hComboBaseMask, CB_SETCURSEL, (WPARAM)s_base_prefix, 0);
        SendMessageW(s_hComboTargetMask, CB_SETCURSEL, (WPARAM)s_target_prefix, 0);
        update_flsm();
    }
}

void tab_flsm_export_csv(HWND hwnd) {
    OPENFILENAMEW ofn = {0};
    wchar_t szFile[MAX_PATH] = L"subnetcalc_flsm.csv";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"csv";

    if (GetSaveFileNameW(&ofn)) {
        FILE *fp = _wfopen(szFile, L"w, ccs=UTF-8");
        if (fp) {
            const char *headers[] = {"Index",        "Network", "Broadcast",
                                     "Usable Range", "Netmask", "Hosts"};
            export_write_csv_row(fp, headers, 6);

            int count = (int)SendMessageW(s_hList, LVM_GETITEMCOUNT, 0, 0);
            for (int i = 0; i < count; i++) {
                wchar_t w0[32], w1[64], w2[64], w3[128], w4[64], w5[32];
                char c0[32], c1[64], c2[64], c3[128], c4[64], c5[32];
                ListView_GetItemText(s_hList, i, 0, w0, 32);
                ListView_GetItemText(s_hList, i, 1, w1, 64);
                ListView_GetItemText(s_hList, i, 2, w2, 64);
                ListView_GetItemText(s_hList, i, 3, w3, 128);
                ListView_GetItemText(s_hList, i, 4, w4, 64);
                ListView_GetItemText(s_hList, i, 5, w5, 32);

                snprintf(c0, sizeof(c0), "%ls", w0);
                snprintf(c1, sizeof(c1), "%ls", w1);
                snprintf(c2, sizeof(c2), "%ls", w2);
                snprintf(c3, sizeof(c3), "%ls", w3);
                snprintf(c4, sizeof(c4), "%ls", w4);
                snprintf(c5, sizeof(c5), "%ls", w5);

                const char *row[] = {c0, c1, c2, c3, c4, c5};
                export_write_csv_row(fp, row, 6);
            }
            fclose(fp);
            MessageBoxW(hwnd, L"FLSM list exported successfully to CSV.", L"SubnetCalc",
                        MB_OK | MB_ICONINFORMATION);
        }
    }
}

static LRESULT CALLBACK tab_flsm_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        if (id == 401 && code == EN_CHANGE) {
            wchar_t text[64];
            GetWindowTextW(s_hEditBaseIP, text, 64);
            char ctext[64];
            snprintf(ctext, sizeof(ctext), "%ls", text);
            uint32_t ip = 0;
            if (ipv4_parse(ctext, &ip)) {
                s_base_ip = ip;
                update_flsm();
            }
        } else if (id == 402 && code == CBN_SELCHANGE) {
            int sel = (int)SendMessageW(s_hComboBaseMask, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel <= 32) {
                s_base_prefix = sel;
                if (s_target_prefix < s_base_prefix) {
                    s_target_prefix = s_base_prefix;
                    SendMessageW(s_hComboTargetMask, CB_SETCURSEL, (WPARAM)s_target_prefix, 0);
                }
                update_flsm();
            }
        } else if (id == 403 && code == CBN_SELCHANGE) {
            int sel = (int)SendMessageW(s_hComboTargetMask, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel <= 32) {
                if (sel < s_base_prefix) {
                    sel = s_base_prefix;
                    SendMessageW(s_hComboTargetMask, CB_SETCURSEL, (WPARAM)sel, 0);
                }
                s_target_prefix = sel;
                update_flsm();
            }
        } else if (id == 404) {
            tab_flsm_export_csv(hwnd);
        }
        break;
    }
    case WM_SIZE: {
        int w = LOWORD(lParam);
        int h = HIWORD(lParam);
        if (s_hList) {
            MoveWindow(s_hList, 15, 80, w - 30, h - 95, TRUE);
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

HWND create_tab_flsm_window(HWND hParent, HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = tab_flsm_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SubnetCalcTabFLSM";
    wc.hbrBackground = theme_brush_bg();
    RegisterClassExW(&wc);

    s_hWnd = CreateWindowExW(0, L"SubnetCalcTabFLSM", L"", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 900,
                             520, hParent, NULL, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Base Network:", WS_CHILD | WS_VISIBLE, 15, 15, 100, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hEditBaseIP = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"192.168.1.0",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP, 120, 13, 130, 24, s_hWnd,
                                    (HMENU)401, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Base Prefix:", WS_CHILD | WS_VISIBLE, 265, 15, 85, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hComboBaseMask = CreateWindowExW(
        0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        355, 13, 110, 250, s_hWnd, (HMENU)402, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Target Prefix:", WS_CHILD | WS_VISIBLE, 480, 15, 95, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hComboTargetMask = CreateWindowExW(
        0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        580, 13, 110, 250, s_hWnd, (HMENU)403, hInstance, NULL);

    for (int p = 0; p <= 32; p++) {
        wchar_t buf[32];
        swprintf(buf, 32, L"/%d", p);
        SendMessageW(s_hComboBaseMask, CB_ADDSTRING, 0, (LPARAM)buf);
        SendMessageW(s_hComboTargetMask, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessageW(s_hComboBaseMask, CB_SETCURSEL, (WPARAM)s_base_prefix, 0);
    SendMessageW(s_hComboTargetMask, CB_SETCURSEL, (WPARAM)s_target_prefix, 0);

    CreateWindowExW(0, L"BUTTON", L"Export CSV", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                    710, 13, 140, 26, s_hWnd, (HMENU)404, hInstance, NULL);

    s_hLblSummary = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 15, 48, 850, 20,
                                    s_hWnd, NULL, hInstance, NULL);

    s_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL, 15,
                              80, 870, 420, s_hWnd, NULL, hInstance, NULL);
    SendMessageW(s_hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    const wchar_t *cols[] = {L"#",       L"Subnet / Prefix", L"Broadcast", L"Usable Range",
                             L"Netmask", L"Usable Hosts"};
    static const int widths[] = {45, 150, 130, 220, 130, 110};

    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    for (int i = 0; i < 6; i++) {
        lvc.pszText = (LPWSTR)cols[i];
        lvc.cx = widths[i];
        lvc.iSubItem = i;
        SendMessageW(s_hList, LVM_INSERTCOLUMNW, (WPARAM)i, (LPARAM)&lvc);
    }

    update_flsm();
    return s_hWnd;
}
