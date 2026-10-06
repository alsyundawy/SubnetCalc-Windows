#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601

#include <windows.h>
#include <commctrl.h>
#include "app.h"
#include "ui/window.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)pCmdLine;

    /* Initialize Common Controls v6 */
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    /* Windows Vista / 7 SP1 DPI Awareness */
    SetProcessDPIAware();

    /* Create Main Window */
    HWND hWnd = create_main_window(hInstance, nCmdShow);
    if (!hWnd) {
        return 1;
    }

    /* Standard Accelerators */
    ACCEL accels[3] = {{FCONTROL | FVIRTKEY, 'C', IDM_EDIT_COPY},
                       {FCONTROL | FVIRTKEY, 'E', IDM_FILE_EXPORT_CSV},
                       {FALT | FVIRTKEY, VK_F4, IDM_FILE_EXIT}};
    HACCEL hAccel = CreateAcceleratorTableW(accels, 3);

    /* Message Loop */
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (!TranslateAcceleratorW(hWnd, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    if (hAccel) {
        DestroyAcceleratorTable(hAccel);
    }

    return (int)msg.wParam;
}
