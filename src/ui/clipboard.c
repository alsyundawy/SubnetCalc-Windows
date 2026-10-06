#include "clipboard.h"

bool clipboard_copy_text(HWND hwnd, const wchar_t *text) {
    if (!text) {
        return false;
    }
    size_t len = wcslen(text);
    size_t bytes = (len + 1) * sizeof(wchar_t);

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) {
        return false;
    }

    wchar_t *pMem = (wchar_t *)GlobalLock(hMem);
    if (!pMem) {
        GlobalFree(hMem);
        return false;
    }
    CopyMemory(pMem, text, bytes);
    GlobalUnlock(hMem);

    if (!OpenClipboard(hwnd)) {
        GlobalFree(hMem);
        return false;
    }

    EmptyClipboard();
    if (!SetClipboardData(CF_UNICODETEXT, hMem)) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}
