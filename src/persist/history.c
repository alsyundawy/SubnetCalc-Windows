#include "history.h"
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>

static wchar_t s_entries[MAX_HISTORY_ENTRIES][HISTORY_ENTRY_MAX_LEN];
static size_t s_count = 0;
static wchar_t s_filepath[MAX_PATH] = {0};

static void get_history_file_path(void) {
    if (s_filepath[0] != L'\0') {
        return;
    }
    wchar_t appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata))) {
        wchar_t dir[MAX_PATH];
        snwprintf(dir, MAX_PATH, L"%s\\SubnetCalc", appdata);
        CreateDirectoryW(dir, NULL);
        snwprintf(s_filepath, MAX_PATH, L"%s\\SubnetCalc\\history.txt", appdata);
    }
}

void history_init(void) {
    get_history_file_path();
    s_count = 0;
    if (s_filepath[0] == L'\0') {
        return;
    }
    FILE *fp = _wfopen(s_filepath, L"r, ccs=UTF-8");
    if (!fp) {
        return;
    }
    wchar_t line[HISTORY_ENTRY_MAX_LEN];
    while (s_count < MAX_HISTORY_ENTRIES && fgetws(line, HISTORY_ENTRY_MAX_LEN, fp)) {
        size_t len = wcslen(line);
        while (len > 0 && (line[len - 1] == L'\r' || line[len - 1] == L'\n')) {
            line[--len] = L'\0';
        }
        if (len > 0) {
            wcsncpy(s_entries[s_count++], line, HISTORY_ENTRY_MAX_LEN - 1);
            s_entries[s_count - 1][HISTORY_ENTRY_MAX_LEN - 1] = L'\0';
        }
    }
    fclose(fp);
}

static void save_history(void) {
    if (s_filepath[0] == L'\0') {
        return;
    }
    FILE *fp = _wfopen(s_filepath, L"w, ccs=UTF-8");
    if (!fp) {
        return;
    }
    for (size_t i = 0; i < s_count; i++) {
        fputws(s_entries[i], fp);
        fputws(L"\n", fp);
    }
    fclose(fp);
}

void history_add(const wchar_t *entry) {
    if (!entry || entry[0] == L'\0') {
        return;
    }
    /* Move to front if exists */
    for (size_t i = 0; i < s_count; i++) {
        if (wcscmp(s_entries[i], entry) == 0) {
            for (size_t j = i; j > 0; j--) {
                wcsncpy(s_entries[j], s_entries[j - 1], HISTORY_ENTRY_MAX_LEN);
            }
            wcsncpy(s_entries[0], entry, HISTORY_ENTRY_MAX_LEN - 1);
            s_entries[0][HISTORY_ENTRY_MAX_LEN - 1] = L'\0';
            save_history();
            return;
        }
    }

    /* Shift right */
    size_t new_count = (s_count < MAX_HISTORY_ENTRIES) ? (s_count + 1) : MAX_HISTORY_ENTRIES;
    for (size_t j = new_count - 1; j > 0; j--) {
        wcsncpy(s_entries[j], s_entries[j - 1], HISTORY_ENTRY_MAX_LEN);
    }
    wcsncpy(s_entries[0], entry, HISTORY_ENTRY_MAX_LEN - 1);
    s_entries[0][HISTORY_ENTRY_MAX_LEN - 1] = L'\0';
    s_count = new_count;
    save_history();
}

void history_clear(void) {
    s_count = 0;
    if (s_filepath[0] != L'\0') {
        FILE *fp = _wfopen(s_filepath, L"w, ccs=UTF-8");
        if (fp) {
            fclose(fp);
        }
    }
}

size_t history_get_count(void) {
    return s_count;
}

const wchar_t *history_get_at(size_t index) {
    if (index < s_count) {
        return s_entries[index];
    }
    return NULL;
}
