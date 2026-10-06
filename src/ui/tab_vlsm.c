#include "tab_vlsm.h"
#include "theme.h"
#include "engine/ipv4.h"
#include "engine/split.h"
#include "engine/export.h"
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>

static HWND s_hWnd = NULL;
static HWND s_hEditBaseIP = NULL;
static HWND s_hComboBaseMask = NULL;
static HWND s_hEditName = NULL;
static HWND s_hEditNeeded = NULL;
static HWND s_hBtnAdd = NULL;
static HWND s_hBtnClear = NULL;
static HWND s_hBtnCalc = NULL;
static HWND s_hBtnExport = NULL;
static HWND s_hListReqs = NULL;
static HWND s_hListResults = NULL;
static HWND s_hLblEfficiency = NULL;

static uint32_t s_base_ip = 0xC0A80100;
static int s_base_prefix = 24;

static vlsm_req_t s_requirements[16];
static size_t s_req_count = 0;

static void update_req_list(void) {
    SendMessageW(s_hListReqs, LVM_DELETEALLITEMS, 0, 0);
    for (size_t i = 0; i < s_req_count; i++) {
        wchar_t wname[64], wneeded[32];
        swprintf(wname, 64, L"%hs", s_requirements[i].name);
        swprintf(wneeded, 32, L"%u", s_requirements[i].needed_hosts);

        LVITEMW lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        lvi.pszText = wname;
        SendMessageW(s_hListReqs, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
        ListView_SetItemText(s_hListReqs, i, 1, wneeded);
    }
}

static void calculate_vlsm_ui(void) {
    if (s_req_count == 0)
        return;

    vlsm_summary_t sum;
    vlsm_calculate(s_base_ip, s_base_prefix, s_requirements, s_req_count, &sum);

    SendMessageW(s_hListResults, LVM_DELETEALLITEMS, 0, 0);
    for (size_t i = 0; i < s_req_count; i++) {
        char net[32], bcast[32], range[64];
        char u_start[32], u_end[32];
        ipv4_format(s_requirements[i].network, net, sizeof(net));
        ipv4_format(s_requirements[i].broadcast, bcast, sizeof(bcast));
        ipv4_format(s_requirements[i].usable_start, u_start, sizeof(u_start));
        ipv4_format(s_requirements[i].usable_end, u_end, sizeof(u_end));
        snprintf(range, sizeof(range), "%s - %s", u_start, u_end);

        wchar_t wbuf[128];
        LVITEMW lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;

        swprintf(wbuf, 128, L"%hs", s_requirements[i].name);
        lvi.pszText = wbuf;
        SendMessageW(s_hListResults, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

        swprintf(wbuf, 128, L"%u", s_requirements[i].needed_hosts);
        ListView_SetItemText(s_hListResults, i, 1, wbuf);

        swprintf(wbuf, 128, L"/%d", s_requirements[i].allocated_prefix);
        ListView_SetItemText(s_hListResults, i, 2, wbuf);

        swprintf(wbuf, 128, L"%hs", net);
        ListView_SetItemText(s_hListResults, i, 3, wbuf);

        swprintf(wbuf, 128, L"%hs", bcast);
        ListView_SetItemText(s_hListResults, i, 4, wbuf);

        swprintf(wbuf, 128, L"%hs", range);
        ListView_SetItemText(s_hListResults, i, 5, wbuf);

        swprintf(wbuf, 128, L"%u", s_requirements[i].allocated_hosts);
        ListView_SetItemText(s_hListResults, i, 6, wbuf);

        swprintf(wbuf, 128, L"%u", s_requirements[i].wasted_hosts);
        ListView_SetItemText(s_hListResults, i, 7, wbuf);
    }

    wchar_t eff_text[256];
    if (sum.fits) {
        swprintf(eff_text, 256,
                 L"VLSM Allocation Status: FITS (Efficiency: %.1f%% — Requested: %u, Allocated: "
                 L"%u, Wasted: %u)",
                 sum.efficiency_percent, sum.total_requested, sum.total_allocated,
                 sum.total_wasted);
    } else {
        swprintf(eff_text, 256,
                 L"VLSM Allocation Status: DOES NOT FIT BASE PREFIX! (Need larger base block)");
    }
    SetWindowTextW(s_hLblEfficiency, eff_text);
}

void tab_vlsm_update(uint32_t ip, int prefix) {
    s_base_ip = ip;
    s_base_prefix = prefix;
    if (s_hEditBaseIP) {
        wchar_t buf[64];
        swprintf(buf, 64, L"%u.%u.%u.%u", (s_base_ip >> 24) & 0xFF, (s_base_ip >> 16) & 0xFF,
                 (s_base_ip >> 8) & 0xFF, s_base_ip & 0xFF);
        SetWindowTextW(s_hEditBaseIP, buf);
        SendMessageW(s_hComboBaseMask, CB_SETCURSEL, (WPARAM)s_base_prefix, 0);
        calculate_vlsm_ui();
    }
}

void tab_vlsm_export_csv(HWND hwnd) {
    OPENFILENAMEW ofn = {0};
    wchar_t szFile[MAX_PATH] = L"subnetcalc_vlsm.csv";
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
            const char *headers[] = {"Name",      "Needed",       "Prefix",    "Network",
                                     "Broadcast", "Usable Range", "Allocated", "Wasted"};
            export_write_csv_row(fp, headers, 8);

            int count = (int)SendMessageW(s_hListResults, LVM_GETITEMCOUNT, 0, 0);
            for (int i = 0; i < count; i++) {
                char cols[8][128];
                const char *row[8];
                for (int c = 0; c < 8; c++) {
                    wchar_t w[128];
                    ListView_GetItemText(s_hListResults, i, c, w, 128);
                    snprintf(cols[c], 128, "%ls", w);
                    row[c] = cols[c];
                }
                export_write_csv_row(fp, row, 8);
            }
            fclose(fp);
            MessageBoxW(hwnd, L"VLSM list exported successfully to CSV.", L"SubnetCalc",
                        MB_OK | MB_ICONINFORMATION);
        }
    }
}

static LRESULT CALLBACK tab_vlsm_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        if (id == 501) { /* Add Requirement */
            wchar_t wname[64], wneeded[32];
            GetWindowTextW(s_hEditName, wname, 64);
            GetWindowTextW(s_hEditNeeded, wneeded, 32);
            int needed = _wtoi(wneeded);
            if (needed > 0 && s_req_count < 16) {
                char cname[64];
                snprintf(cname, 64, "%ls", wname);
                strncpy(s_requirements[s_req_count].name, (cname[0] != '\0') ? cname : "Subnet",
                        63);
                s_requirements[s_req_count].needed_hosts = (uint32_t)needed;
                s_req_count++;
                update_req_list();
                calculate_vlsm_ui();
                SetWindowTextW(s_hEditName, L"");
                SetWindowTextW(s_hEditNeeded, L"");
            }
        } else if (id == 502) { /* Clear All */
            s_req_count = 0;
            update_req_list();
            SendMessageW(s_hListResults, LVM_DELETEALLITEMS, 0, 0);
            SetWindowTextW(s_hLblEfficiency, L"");
        } else if (id == 503) { /* Calculate */
            calculate_vlsm_ui();
        } else if (id == 504) { /* Export */
            tab_vlsm_export_csv(hwnd);
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

HWND create_tab_vlsm_window(HWND hParent, HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = tab_vlsm_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SubnetCalcTabVLSM";
    wc.hbrBackground = theme_brush_bg();
    RegisterClassExW(&wc);

    s_hWnd = CreateWindowExW(0, L"SubnetCalcTabVLSM", L"", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 900,
                             520, hParent, NULL, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Base Network:", WS_CHILD | WS_VISIBLE, 15, 15, 100, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hEditBaseIP = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"192.168.1.0",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP, 120, 13, 130, 24, s_hWnd,
                                    NULL, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Prefix:", WS_CHILD | WS_VISIBLE, 265, 15, 45, 20, s_hWnd, NULL,
                    hInstance, NULL);
    s_hComboBaseMask = CreateWindowExW(
        0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        315, 13, 100, 250, s_hWnd, NULL, hInstance, NULL);
    for (int p = 0; p <= 32; p++) {
        wchar_t buf[32];
        swprintf(buf, 32, L"/%d", p);
        SendMessageW(s_hComboBaseMask, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessageW(s_hComboBaseMask, CB_SETCURSEL, (WPARAM)s_base_prefix, 0);

    /* Requirement Inputs */
    CreateWindowExW(0, L"STATIC", L"Subnet Name:", WS_CHILD | WS_VISIBLE, 15, 48, 90, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hEditName = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Engineering",
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP, 105, 46, 120, 24, s_hWnd,
                                  NULL, hInstance, NULL);

    CreateWindowExW(0, L"STATIC", L"Needed Hosts:", WS_CHILD | WS_VISIBLE, 235, 48, 95, 20, s_hWnd,
                    NULL, hInstance, NULL);
    s_hEditNeeded = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"50",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 335, 46, 70, 24,
                                    s_hWnd, NULL, hInstance, NULL);

    s_hBtnAdd = CreateWindowExW(0, L"BUTTON", L"Add Requirement",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 420, 45, 130,
                                26, s_hWnd, (HMENU)501, hInstance, NULL);
    s_hBtnClear = CreateWindowExW(0, L"BUTTON", L"Clear All",
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 560, 45, 90,
                                  26, s_hWnd, (HMENU)502, hInstance, NULL);
    s_hBtnCalc = CreateWindowExW(0, L"BUTTON", L"Calculate VLSM",
                                 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 660, 45, 120,
                                 26, s_hWnd, (HMENU)503, hInstance, NULL);
    s_hBtnExport = CreateWindowExW(0, L"BUTTON", L"Export CSV",
                                   WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 790, 45, 95,
                                   26, s_hWnd, (HMENU)504, hInstance, NULL);

    /* Requirements List */
    s_hListReqs = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL,
                                  15, 80, 260, 390, s_hWnd, NULL, hInstance, NULL);
    SendMessageW(s_hListReqs, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                 LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    LVCOLUMNW lvc1 = {LVCF_TEXT | LVCF_WIDTH, 0, 150, L"Name", 0, 0, 0, 0};
    SendMessageW(s_hListReqs, LVM_INSERTCOLUMNW, 0, (LPARAM)&lvc1);
    LVCOLUMNW lvc2 = {LVCF_TEXT | LVCF_WIDTH, 0, 90, L"Needed", 0, 1, 0, 0};
    SendMessageW(s_hListReqs, LVM_INSERTCOLUMNW, 1, (LPARAM)&lvc2);

    /* Results List */
    s_hListResults =
        CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL, 290, 80,
                        595, 390, s_hWnd, NULL, hInstance, NULL);
    SendMessageW(s_hListResults, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                 LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    const wchar_t *res_cols[] = {L"Name",      L"Needed",       L"Prefix", L"Network",
                                 L"Broadcast", L"Usable Range", L"Alloc",  L"Waste"};
    static const int res_widths[] = {80, 55, 45, 95, 95, 140, 50, 45};
    for (int i = 0; i < 8; i++) {
        LVCOLUMNW lvc = {LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM,
                         0,
                         res_widths[i],
                         (LPWSTR)res_cols[i],
                         0,
                         i,
                         0,
                         0};
        SendMessageW(s_hListResults, LVM_INSERTCOLUMNW, (WPARAM)i, (LPARAM)&lvc);
    }

    s_hLblEfficiency = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 15, 480, 870, 20,
                                       s_hWnd, NULL, hInstance, NULL);

    /* Add standard initial sample */
    strncpy(s_requirements[0].name, "Engineering", 63);
    s_requirements[0].needed_hosts = 60;
    strncpy(s_requirements[1].name, "Sales", 63);
    s_requirements[1].needed_hosts = 25;
    strncpy(s_requirements[2].name, "Servers", 63);
    s_requirements[2].needed_hosts = 10;
    s_req_count = 3;
    update_req_list();
    calculate_vlsm_ui();

    return s_hWnd;
}
