#include "engine/platform/platform.h"

#ifdef _WIN32

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PlatformState* state = (PlatformState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (uMsg) {
        case WM_CLOSE:
            state->display.shouldClose = true;
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            state->display.width = LOWORD(lParam);
            state->display.height = HIWORD(lParam);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

Result window_init(PlatformInitConfig info, PlatformState* out) {
    out->display.shouldClose = false;
    out->display.width = info.display.w;
    out->display.height = info.display.h;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "GameWindowClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        "GameWindowClass",
        "Game",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, info.display.w, info.display.h,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        return RESULT_CODE_FAILED_DISPLAY_OPENING;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)out);
    ShowWindow(hwnd, SW_SHOW);

    out->display.hwnd = hwnd;
    out->display.hInstance = hInstance;

    return RESULT_CODE_SUCCESS;
}

Result window_destroy(PlatformState *state) {
    DestroyWindow(state->display.hwnd);
    return RESULT_CODE_SUCCESS;
}

Result window_PullEvents(PlatformState* state) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return RESULT_CODE_SUCCESS;
}

#endif