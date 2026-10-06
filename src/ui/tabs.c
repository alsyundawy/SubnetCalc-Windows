#include "tabs.h"
#include "tab_ipv4.h"
#include "tab_hosts.h"
#include "tab_cidr.h"
#include "tab_flsm.h"
#include "tab_vlsm.h"
#include "tab_ipv6.h"

static HWND s_hTabWnds[TAB_COUNT] = {NULL};
static tab_index_t s_active_tab = TAB_IPV4;

HWND create_main_tabs(HWND hParent, HINSTANCE hInstance) {
    HWND hTabCtrl = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                    10, 10, 920, 560, hParent, (HMENU)100, hInstance, NULL);

    const wchar_t *tab_titles[TAB_COUNT] = {L"IPv4 Subnet", L"Subnets / Hosts", L"CIDR", L"FLSM",
                                            L"VLSM",        L"IPv6 Subnet"};

    TCITEMW tie;
    tie.mask = TCIF_TEXT;
    for (int i = 0; i < TAB_COUNT; i++) {
        tie.pszText = (LPWSTR)tab_titles[i];
        SendMessageW(hTabCtrl, TCM_INSERTITEMW, (WPARAM)i, (LPARAM)&tie);
    }

    /* Create child tab pages */
    s_hTabWnds[TAB_IPV4] = create_tab_ipv4_window(hTabCtrl, hInstance);
    s_hTabWnds[TAB_HOSTS] = create_tab_hosts_window(hTabCtrl, hInstance);
    s_hTabWnds[TAB_CIDR] = create_tab_cidr_window(hTabCtrl, hInstance);
    s_hTabWnds[TAB_FLSM] = create_tab_flsm_window(hTabCtrl, hInstance);
    s_hTabWnds[TAB_VLSM] = create_tab_vlsm_window(hTabCtrl, hInstance);
    s_hTabWnds[TAB_IPV6] = create_tab_ipv6_window(hTabCtrl, hInstance);

    select_tab(hTabCtrl, TAB_IPV4);
    return hTabCtrl;
}

void resize_main_tabs(HWND hTabCtrl, int x, int y, int width, int height) {
    MoveWindow(hTabCtrl, x, y, width, height, TRUE);

    RECT rc;
    GetClientRect(hTabCtrl, &rc);
    SendMessageW(hTabCtrl, TCM_ADJUSTRECT, FALSE, (LPARAM)&rc);

    for (int i = 0; i < TAB_COUNT; i++) {
        if (s_hTabWnds[i]) {
            MoveWindow(s_hTabWnds[i], rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                       TRUE);
        }
    }
}

void select_tab(HWND hTabCtrl, tab_index_t tab) {
    if (tab >= TAB_COUNT)
        return;
    s_active_tab = tab;
    SendMessageW(hTabCtrl, TCM_SETCURSEL, (WPARAM)tab, 0);

    for (int i = 0; i < TAB_COUNT; i++) {
        if (s_hTabWnds[i]) {
            ShowWindow(s_hTabWnds[i], (i == tab) ? SW_SHOW : SW_HIDE);
        }
    }

    /* Sync data from tab_ipv4 to hosts, flsm, vlsm, cidr when switching */
    uint32_t cur_ip = 0;
    int cur_prefix = 24;
    tab_ipv4_get_current_state(&cur_ip, &cur_prefix);

    if (tab == TAB_HOSTS) {
        tab_hosts_update(cur_ip, cur_prefix);
    } else if (tab == TAB_FLSM) {
        tab_flsm_update(cur_ip, cur_prefix);
    } else if (tab == TAB_VLSM) {
        tab_vlsm_update(cur_ip, cur_prefix);
    } else if (tab == TAB_CIDR) {
        tab_cidr_update(cur_ip, cur_prefix);
    }
}

tab_index_t get_current_tab(HWND hTabCtrl) {
    return (tab_index_t)SendMessageW(hTabCtrl, TCM_GETCURSEL, 0, 0);
}

HWND get_tab_window(tab_index_t tab) {
    if (tab < TAB_COUNT) {
        return s_hTabWnds[tab];
    }
    return NULL;
}
