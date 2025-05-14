#include <windows.h>
#include <commctrl.h> // For SetWindowSubclass
#include <iostream>
#include <string>

LRESULT CALLBACK MPVSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                 UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // Store parent window for focus management
    HWND parentWindow = GetParent(hWnd);

    // For hit testing, return HTTRANSPARENT instead of HTCLIENT
    // This is critical - it makes mouse events pass through to parent
    if (uMsg == WM_NCHITTEST)
    {
        return HTTRANSPARENT; // Pass through to parent window
    }

    // Block ALL mouse-related messages completely
    switch (uMsg)
    {
    // Focus and activation blocking
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
    case WM_NCACTIVATE:
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
    case WM_CHILDACTIVATE:
        if (parentWindow)
            SetFocus(parentWindow); // Force focus back to parent
        return 0;

    // Mouse interaction blocking
    case WM_NCLBUTTONDOWN:
    case WM_NCRBUTTONDOWN:
    case WM_NCMBUTTONDOWN:
    case WM_NCLBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_MOUSEMOVE:
    case WM_MOUSEWHEEL:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_MOUSEHOVER:
    case WM_MOUSELEAVE:
    case WM_NCMOUSEMOVE:
    case WM_NCLBUTTONUP:
    case WM_NCRBUTTONUP:
    case WM_NCMBUTTONUP:
    case WM_SETCURSOR:     // Block cursor changes
    case WM_MOUSEACTIVATE: // Block mouse activation
        if (parentWindow)
            SetFocus(parentWindow); // Force focus back to parent
        return 0;                   // Block these messages completely

    // Block window movement messages directly
    case WM_MOVING:
    case WM_MOVE:
    case WM_SIZING:
    case WM_SIZE: // Block size changes
    case WM_WINDOWPOSCHANGING:
    case WM_WINDOWPOSCHANGED: // Block position changes
    case WM_ENTERSIZEMOVE:
    case WM_EXITSIZEMOVE:
    case WM_CAPTURECHANGED:
    case WM_NCHITTEST: // Block hit testing (duplicated for clarity)
        return 0;      // Block completely, don't pass to DefSubclassProc

    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MOVE || (wParam & 0xFFF0) == SC_SIZE ||
            (wParam & 0xFFF0) == SC_MOUSEMENU || (wParam & 0xFFF0) == SC_KEYMENU)
            return 0;
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

int main(int argc, char *argv[])
{
    INITCOMMONCONTROLSEX icex = {sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icex);

    if (argc < 3)
    {
        std::cerr << "Usage: window-merger.exe <mpv_window_title> <electron_window_title>\n";
        return 1;
    }

    std::string mpvTitle = argv[1];
    std::string electronTitle = argv[2];

    HWND mpvWindow = FindWindowA(NULL, mpvTitle.c_str());
    HWND electronWindow = FindWindowA(NULL, electronTitle.c_str());

    if (!mpvWindow)
    {
        std::cerr << "Could not find MPV window with title: " << mpvTitle << "\n";
        return 2;
    }
    if (!electronWindow)
    {
        std::cerr << "Could not find Electron window with title: " << electronTitle << "\n";
        return 3;
    }

    SetParent(mpvWindow, electronWindow);

    EnableWindow(mpvWindow, FALSE);

    SetFocus(electronWindow);

    if (!SetWindowSubclass(mpvWindow, MPVSubclassProc, 1, 0))
    {
        std::cerr << "Failed to subclass MPV window. Error: " << GetLastError() << std::endl;
    }

    LONG style = GetWindowLongA(mpvWindow, GWL_STYLE);
    // Remove ALL window decoration and interaction styles
    style &= ~(WS_CAPTION | WS_POPUP | WS_SIZEBOX | WS_THICKFRAME | WS_DLGFRAME | WS_SYSMENU |
               WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_BORDER | WS_VSCROLL | WS_HSCROLL |
               WS_TABSTOP | WS_GROUP);
    // Add ONLY child and visible styles
    style |= WS_CHILD | WS_VISIBLE | WS_DISABLED;
    SetWindowLongA(mpvWindow, GWL_STYLE, style);

    LONG exStyle = GetWindowLongA(mpvWindow, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE |
                 WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME | WS_EX_STATICEDGE | WS_EX_TOOLWINDOW |
                 WS_EX_CONTROLPARENT | WS_EX_APPWINDOW | WS_EX_ACCEPTFILES);
    exStyle |= WS_EX_NOACTIVATE; // Add the no-activate style to prevent activation
    SetWindowLongA(mpvWindow, GWL_EXSTYLE, exStyle);

    SetActiveWindow(electronWindow);

    RECT rect;
    GetClientRect(electronWindow, &rect);
    SetWindowPos(mpvWindow, HWND_TOP, 0, 0, rect.right, rect.bottom, SWP_SHOWWINDOW);

    // Now show the MPV window after all manipulation is done
    RedrawWindow(mpvWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    SendMessage(mpvWindow, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.right, rect.bottom));

    RECT lastRect = rect;
    while (IsWindow(mpvWindow) && IsWindow(electronWindow))
    {
        Sleep(50); // Poll every 50ms (adjust as needed)
        RECT newRect;
        GetClientRect(electronWindow, &newRect);
        if (newRect.right != lastRect.right || newRect.bottom != lastRect.bottom)
        {
            ShowWindow(mpvWindow, SW_SHOW);
            SetWindowPos(mpvWindow, NULL, 0, 0, newRect.right, newRect.bottom,
                         SWP_NOZORDER | SWP_SHOWWINDOW);
            RedrawWindow(mpvWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            SendMessage(mpvWindow, WM_SIZE, SIZE_RESTORED, MAKELPARAM(newRect.right, newRect.bottom));
            lastRect = newRect;
        }
    }
    // --- End persistent resize loop ---

    return 0;
}