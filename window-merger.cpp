#include <windows.h>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
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

    // Set MPV as child of Electron window
    SetParent(mpvWindow, electronWindow);

    // Remove window border and title bar, set as child
    LONG style = GetWindowLongA(mpvWindow, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_POPUP | WS_SYSMENU); // Only remove border and caption, keep resizing
    style |= WS_CHILD | WS_VISIBLE | WS_THICKFRAME | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLongA(mpvWindow, GWL_STYLE, style);

    LONG sysMenuStyle = GetWindowLongA(mpvWindow, GWL_STYLE);
    sysMenuStyle |= (WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    SetWindowLongA(mpvWindow, GWL_STYLE, sysMenuStyle);
    // Optionally restore menu if needed (commented out):
    // SetMenu(mpvWindow, GetSystemMenu(mpvWindow, TRUE));

    // Resize MPV to exactly fill the Electron window
    RECT rect;
    GetClientRect(electronWindow, &rect);
    SetWindowPos(mpvWindow, NULL, 0, 0, rect.right, rect.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);

    // After resizing MPV to fill parent, force a redraw
    RedrawWindow(mpvWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

    // Also send a WM_SIZE message to MPV to force it to re-layout video
    SendMessage(mpvWindow, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.right, rect.bottom));

    // --- Begin persistent resize loop ---
    RECT lastRect = rect;
    while (IsWindow(mpvWindow) && IsWindow(electronWindow)) {
        Sleep(50); // Poll every 50ms (adjust as needed)
        RECT newRect;
        GetClientRect(electronWindow, &newRect);
        if (newRect.right != lastRect.right || newRect.bottom != lastRect.bottom) {
            SetWindowPos(mpvWindow, NULL, 0, 0, newRect.right, newRect.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
            RedrawWindow(mpvWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            SendMessage(mpvWindow, WM_SIZE, SIZE_RESTORED, MAKELPARAM(newRect.right, newRect.bottom));
            lastRect = newRect;
        }
    }
    // --- End persistent resize loop ---

    return 0;
}