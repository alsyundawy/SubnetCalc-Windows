#include "tab_hosts.h"
#include "theme.h"
#include "engine/ipv4.h"
#include "engine/split.h"
#include "engine/export.h"
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>

static HWND s_hWnd = NULL;
static HWND s_hList = NULL;
static HWND s_hBtnExportCSV = NULL;
static HWND s_hBtnExportASCII = NULL;
static uint32_t s_current_ip = 0xC0A80100;
static int s_current_prefix = 24;

static void populate_list(void) {
    SendMessageW(s_hList, LVM_DELETEALLITEMS, 0, 0);

    /* Generate subnets for current base */
    subnet_slice_t slices[256];
    int target_p = (s_current_prefix <= 24) ? (s_current_prefix + 2) : s_current_prefix;
    if (target_p > 32)
        target_p = 32;

    size_t count = flsm_split(s_current_ip, s_current_prefix, target_p, slices, 256);

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
}

void tab_hosts_update(uint32_t ip, int prefix) {
    s_current_ip = ip;
    s_current_prefix = prefix;
    if (s_hList) {
        populate_list();
    }
}

void tab_hosts_export_csv(HWND hwnd) {
    OPENFILENAMEW ofn = {0};
    wchar_t szFile[MAX_PATH] = L"subnetcalc_hosts.csv";
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
            MessageBoxW(hwnd, L"Subnet list exported successfully to CSV.", L"SubnetCalc",
                        MB_OK | MB_ICONINFORMATION);
        }
    }
}

void tab_hosts_export_ascii(HWND hwnd) {
    OPENFILENAMEW ofn = {0};
    wchar_t szFile[MAX_PATH] = L"subnetcalc_hosts.txt";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"txt";

    if (GetSaveFileNameW(&ofn)) {
        FILE *fp = _wfopen(szFile, L"w, ccs=UTF-8");
        if (fp) {
            const char *headers[] = {"Index",        "Network", "Broadcast",
                                     "Usable Range", "Netmask", "Hosts"};
            int count = (int)SendMessageW(s_hList, LVM_GETITEMCOUNT, 0, 0);

            if (count <= 0) {
                fclose(fp);
                return;
            }

            /* Allocate temporary row storage */
            char ***rows = (char ***)malloc((size_t)count * sizeof(char **));
            if (!rows) {
                fclose(fp);
                return;
            }

            bool alloc_failed = false;
            int allocated_count = 0;
            for (int i = 0; i < count; i++) {
                rows[i] = (char **)malloc(6 * sizeof(char *));
                if (!rows[i]) {
                    alloc_failed = true;
                    break;
                }
                allocated_count++;
                for (int c = 0; c < 6; c++) {
                    wchar_t w[128];
                    ListView_GetItemText(s_hList, i, c, w, 128);
                    rows[i][c] = (char *)malloc(128);
                    if (!rows[i][c]) {
                        alloc_failed = true;
                        break;
                    }
                    snprintf(rows[i][c], 128, "%ls", w);
                }
                if (alloc_failed) {
                    break;
                }
            }

            if (!alloc_failed) {
                export_write_ascii_table(fp, headers, 6, (const char *const *const *)rows,
                                         (size_t)count);
            }

            for (int i = 0; i < allocated_count; i++) {
                if (rows[i]) {
                    for (int c = 0; c < 6; c++) {
                        if (rows[i][c]) {
                            free(rows[i][c]);
                        }
                    }
                    free(rows[i]);
                }
            }
            free(rows);
            fclose(fp);
            if (!alloc_failed) {
                MessageBoxW(hwnd, L"Subnet list exported successfully to ASCII table.",
                            L"SubnetCalc", MB_OK | MB_ICONINFORMATION);
            }
        }
    }
}

static LRESULT CALLBACK tab_hosts_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        if (id == 201) {
            tab_hosts_export_csv(hwnd);
        } else if (id == 202) {
            tab_hosts_export_ascii(hwnd);
        }
        break;
    }
    case WM_SIZE: {
        int w = LOWORD(lParam);
        int h = HIWORD(lParam);
        if (s_hList) {
            MoveWindow(s_hList, 15, 50, w - 30, h - 65, TRUE);
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

HWND create_tab_hosts_window(HWND hParent, HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = tab_hosts_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SubnetCalcTabHosts";
    wc.hbrBackground = theme_brush_bg();
    RegisterClassExW(&wc);

    s_hWnd = CreateWindowExW(0, L"SubnetCalcTabHosts", L"", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 900,
                             520, hParent, NULL, hInstance, NULL);

    s_hBtnExportCSV = CreateWindowExW(0, L"BUTTON", L"Export CSV (Ctrl+E)",
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 15, 12,
                                      160, 26, s_hWnd, (HMENU)201, hInstance, NULL);
    s_hBtnExportASCII = CreateWindowExW(0, L"BUTTON", L"Export ASCII Table",
                                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 185, 12,
                                        160, 26, s_hWnd, (HMENU)202, hInstance, NULL);

    s_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL, 15,
                              50, 870, 450, s_hWnd, (HMENU)203, hInstance, NULL);
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

    populate_list();
    return s_hWnd;
}
