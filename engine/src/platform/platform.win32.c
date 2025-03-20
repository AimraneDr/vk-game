#include "platform/platform.h"

#ifdef _WIN32

#include "core/debugger.h"
#include "core/events.h"
#include "core/input.h"

#include <string/str.h>

Key map_windows_keycode(u8 vk_code);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PlatformState* state = (PlatformState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    static bool is_tracking_mouse_leave = false;

    switch (uMsg) {
        case WM_CLOSE:
            state->display.shouldClose = true;
            PostQuitMessage(0);
            emit_event(EVENT_TYPE_WINDOW_CLOSED, (EventContext){0},state);
            break;
        case WM_ACTIVATEAPP:
            if(wParam){
                emit_event(EVENT_TYPE_WINDOW_ACTIVATED,(EventContext){0}, state);
            }else{
                emit_event(EVENT_TYPE_WINDOW_DEACTIVATED,(EventContext){0}, state);
            }
            break;
        case WM_ENTERSIZEMOVE:{
            state->display.isResizing = true;
            break;
        }
        case WM_SIZE:
        {
            bool changed = state->display.width != LOWORD(lParam) || state->display.height != HIWORD(lParam);
            state->display.width = LOWORD(lParam);
            state->display.height = HIWORD(lParam);
        
            switch (wParam)
            {
                case SIZE_MINIMIZED:
                {
                    state->display.visibility = WINDOW_STATE_MINIMIZED;
                    emit_event(EVENT_TYPE_WINDOW_MINIMIZED, (EventContext){0}, state);
                    break;
                }
                case SIZE_MAXIMIZED:
                {
                    state->display.visibility = WINDOW_STATE_MAXIMIZED;
                    // Immediate resize for maximize
                    EventContext context = {
                        .u32[0] = state->display.width,
                        .u32[1] = state->display.height
                    };
                    emit_event(EVENT_TYPE_WINDOW_RESIZED, context, state);
                    emit_event(EVENT_TYPE_WINDOW_MAXIMIZED, (EventContext){0}, state);
                    break;
                }
                case SIZE_RESTORED:
                {
                    // Handle restore from minimized/maximized
                    if (state->display.visibility == WINDOW_STATE_MINIMIZED)
                    {
                        // Coming from minimized - special handling if needed
                        state->display.visibility = WINDOW_STATE_FLOATING;
                    }
                    else if(state->display.visibility == WINDOW_STATE_MAXIMIZED)
                    {
                        state->display.visibility = WINDOW_STATE_FLOATING;
                        // Only emit if dimensions actually changed
                        if (changed)
                        {
                            EventContext context = {
                                .u32[0] = state->display.width,
                                .u32[1] = state->display.height
                            };
                            emit_event(EVENT_TYPE_WINDOW_RESIZED, context, state);
                        }
                    }
                    break;
                }
            }
            break;
        }
        case WM_SIZING:
        {
            // Update dimensions during resize
            RECT* rect = (RECT*)lParam;
            state->display.width = rect->right - rect->left;
            state->display.height = rect->bottom - rect->top;
            
            EventContext context = {
                .u32[0] = state->display.width,
                .u32[1] = state->display.height
            };
            emit_event(EVENT_TYPE_WINDOW_RESIZING, context, state);
            break;
        }
        case WM_EXITSIZEMOVE:
        {
            if (state->display.isResizing)
            {
                EventContext context = {
                    .u32[0] = state->display.width,
                    .u32[1] = state->display.height
                };
                emit_event(EVENT_TYPE_WINDOW_RESIZED, context, state);
            }
            state->display.isResizing = false;
            
            // Additional check for maximize/restore via titlebar buttons
            if (state->display.visibility == WINDOW_STATE_MAXIMIZED ||
                state->display.visibility == WINDOW_STATE_FLOATING)
            {
                EventContext context = {
                    .u32[0] = state->display.width,
                    .u32[1] = state->display.height
                };
                emit_event(EVENT_TYPE_WINDOW_RESIZED, context, state);
            }
            break;
        }
        //Keyboard
        case WM_KEYDOWN:{
                EventContext context = {
                    .u8[0] = map_windows_keycode(wParam),
                };
                emit_event(EVENT_TYPE_KEY_DOWN, context, state);  
            }
            break;
        case WM_KEYUP:{
                EventContext context = {
                    .u8[0] = map_windows_keycode(wParam),
                };
                emit_event(EVENT_TYPE_KEY_UP, context, state);
            }
            break;
        // Mouse Movement
        case WM_MOUSEMOVE:{
                EventContext context = {
                    .i16[0] = (i16)LOWORD(lParam),
                    .i16[1] = (i16)HIWORD(lParam)
                };
                emit_event(EVENT_TYPE_MOUSE_MOVED, context, state);

                if(!is_tracking_mouse_leave){
                    emit_event(EVENT_TYPE_MOUSE_ENTER_WINDOW, context, state);

                    TRACKMOUSEEVENT tme;
                    tme.cbSize = sizeof(TRACKMOUSEEVENT);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hwnd;
                    TrackMouseEvent(&tme);
                    is_tracking_mouse_leave = true;
                }
            }
            break;
        case WM_MOUSELEAVE:{
                is_tracking_mouse_leave = false;
                emit_event(EVENT_TYPE_MOUSE_LEAVE_WINDOW, (EventContext){0}, state);
            }
            break;
        // Mouse Buttons
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: 
        case WM_XBUTTONDOWN: {
            Key button = KEY_NULL;
                switch (uMsg) {
                    case WM_LBUTTONDOWN: button = MOUSE_BUTTON_LEFT; break; // Left
                    case WM_RBUTTONDOWN: button = MOUSE_BUTTON_RIGHT; break; // Right
                    case WM_MBUTTONDOWN: button = MOUSE_BUTTON_MIDDLE; break; // Middle
                    case WM_XBUTTONDOWN:
                        button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? MOUSE_BUTTON_0 : MOUSE_BUTTON_1;
                        break;
                }
                EventContext context = {
                    .u8[0] = (u8)button,
                };
                emit_event(EVENT_TYPE_MOUSE_BUTTON_DOWN, context, state);
            }
            break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP: {
                Key button = KEY_NULL;
                switch (uMsg) {
                    case WM_LBUTTONUP: button = MOUSE_BUTTON_LEFT; break; // Left
                    case WM_RBUTTONUP: button = MOUSE_BUTTON_RIGHT; break; // Right
                    case WM_MBUTTONUP: button = MOUSE_BUTTON_MIDDLE; break; // Middle
                    case WM_XBUTTONUP:
                        button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? MOUSE_BUTTON_0 : MOUSE_BUTTON_1;
                        break;
                }
                EventContext context = {
                    .u8[0] = (u8)button,
                };
                emit_event(EVENT_TYPE_MOUSE_BUTTON_UP, context, state);
            }
            break;
        // Mouse Scroll
        case WM_MOUSEWHEEL:{ 
                i16 delta = (i16)(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
                EventContext context = {
                    .i16[0] = delta,
                };
                emit_event(EVENT_TYPE_MOUSE_SCROLL, context, state);
            }
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void window_init(PlatformInitConfig info, PlatformState* out) {
    out->display.shouldClose = false;
    out->display.isResizing = false;
    out->display.width = info.display.w;
    out->display.height = info.display.h;
    out->display.title = str_new(info.title.val);

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "GameWindowClass";

    RegisterClass(&wc);

    // Calculate window position
    u32 windowX = info.display.x;
    u32 windowY = info.display.y;
    
    // Get screen dimensions for position calculations
    i32 screenWidth = GetSystemMetrics(SM_CXSCREEN);
    i32 screenHeight = GetSystemMetrics(SM_CYSCREEN);

      // Handle X position
      switch (windowX) {
        case WINDOW_START_POSITION_RANDOM:
            windowX = CW_USEDEFAULT;
            break;
        case WINDOW_START_POSITION_END:
            windowX = screenWidth - info.display.w;
            break;
        case WINDOW_START_POSITION_CENTER:
            windowX = (screenWidth - info.display.w) / 2;
            break;
    }

    // Handle Y position
    switch (windowY) {
        case WINDOW_START_POSITION_RANDOM:
            windowY = CW_USEDEFAULT;
            break;
        case WINDOW_START_POSITION_END:
            windowY = screenHeight - info.display.h;
            break;
        case WINDOW_START_POSITION_CENTER:
            windowY = (screenHeight - info.display.h) / 2;
            break;
    }

    DWORD windowStyle = WS_OVERLAPPED | WS_MINIMIZEBOX;
    if (info.display.resizable && info.display.startState != WINDOW_STATE_FULL_SCREEN) {
        windowStyle |= WS_MAXIMIZEBOX | WS_THICKFRAME;
    }
    if (info.display.startState != WINDOW_STATE_FULL_SCREEN) {
        windowStyle |= WS_SYSMENU | WS_CAPTION;
    }

    HWND hwnd = CreateWindowEx(
        0,
        "GameWindowClass",
        info.title.val,
        windowStyle,
        windowX, windowY, info.display.w, info.display.h,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        LOG_ERROR("failed to creat window handle");
        return;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)out);

    // Set initial window state
    int cmdShow = SW_SHOW;
    switch (info.display.startState) {
        case WINDOW_STATE_FULL_SCREEN:
            cmdShow = SW_MAXIMIZE;
            SetWindowLong(hwnd, GWL_STYLE, windowStyle | WS_POPUP);
            SetWindowPos(hwnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_FRAMECHANGED);
            break;
        case WINDOW_STATE_MAXIMIZED:
            cmdShow = SW_MAXIMIZE;
            break;
        case WINDOW_STATE_MINIMIZED:
            cmdShow = SW_MINIMIZE;
            break;
        case WINDOW_STATE_FLOATING:
            cmdShow = SW_SHOW;
            break;
        default:
            break;
    }

    ShowWindow(hwnd, cmdShow);

    out->display.hwnd = hwnd;
    out->display.hInstance = hInstance;

    return;
}

void window_destroy(PlatformState *state) {
    DestroyWindow(state->display.hwnd);
    UnregisterClass("GameWindowClass", state->display.hInstance);
    state->display.hwnd = NULL;
    state->display.hInstance = NULL;
    return;
}

void window_PullEvents(PlatformState* state) {
    MSG msg;
    while (PeekMessage(&msg, state->display.hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return;
}

Key map_windows_keycode(u8 vk_code) {
    switch(vk_code) {
        // Letters (Windows uses ASCII values for these)
        case 0x41: return KEY_A;
        case 0x42: return KEY_B;
        case 0x43: return KEY_C;
        case 0x44: return KEY_D;
        case 0x45: return KEY_E;
        case 0x46: return KEY_F;
        case 0x47: return KEY_G;
        case 0x48: return KEY_H;
        case 0x49: return KEY_I;
        case 0x4A: return KEY_J;
        case 0x4B: return KEY_K;
        case 0x4C: return KEY_L;
        case 0x4D: return KEY_M;
        case 0x4E: return KEY_N;
        case 0x4F: return KEY_O;
        case 0x50: return KEY_P;
        case 0x51: return KEY_Q;
        case 0x52: return KEY_R;
        case 0x53: return KEY_S;
        case 0x54: return KEY_T;
        case 0x55: return KEY_U;
        case 0x56: return KEY_V;
        case 0x57: return KEY_W;
        case 0x58: return KEY_X;
        case 0x59: return KEY_Y;
        case 0x5A: return KEY_Z;

        // Numbers
        case 0x30: return KEY_0;
        case 0x31: return KEY_1;
        case 0x32: return KEY_2;
        case 0x33: return KEY_3;
        case 0x34: return KEY_4;
        case 0x35: return KEY_5;
        case 0x36: return KEY_6;
        case 0x37: return KEY_7;
        case 0x38: return KEY_8;
        case 0x39: return KEY_9;

        // Function keys
        case VK_F1:  return KEY_F1;
        case VK_F2:  return KEY_F2;
        case VK_F3:  return KEY_F3;
        case VK_F4:  return KEY_F4;
        case VK_F5:  return KEY_F5;
        case VK_F6:  return KEY_F6;
        case VK_F7:  return KEY_F7;
        case VK_F8:  return KEY_F8;
        case VK_F9:  return KEY_F9;
        case VK_F10: return KEY_F10;
        case VK_F11: return KEY_F11;
        case VK_F12: return KEY_F12;

        // Modifiers and special keys
        case VK_SHIFT:     return KEY_SHIFT;
        case VK_CONTROL:   return KEY_CONTROL;
        case VK_MENU:      return KEY_ALT;
        case VK_ESCAPE:    return KEY_ESCAPE;
        case VK_SPACE:     return KEY_SPACE;
        case VK_RETURN:    return KEY_ENTER;
        case VK_BACK:      return KEY_BACKSPACE;
        case VK_TAB:       return KEY_TAB;
        case VK_CAPITAL:   return KEY_CAPS_LOCK;

        // Arrow keys
        case VK_UP:        return KEY_UP;
        case VK_DOWN:      return KEY_DOWN;
        case VK_LEFT:      return KEY_LEFT;
        case VK_RIGHT:     return KEY_RIGHT;

        default:           return KEY_NULL;
    }
}

//time
// Static variables for time conversion
static LARGE_INTEGER frequency;
static f64 frequency_inv = 0;


f64 platform_get_time(void) {
    if (!frequency_inv) {
        QueryPerformanceFrequency(&frequency);
        frequency_inv = 1.0 / (f64)frequency.QuadPart;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (f64)counter.QuadPart * frequency_inv;
}

void platform_sleep(f64 seconds){
    if (seconds <= 0) return;
    
    const f64 start = platform_get_time();
    while (platform_get_time() - start < seconds) {
        // Yield CPU to other threads
        Sleep(0);
    }
}

#endif