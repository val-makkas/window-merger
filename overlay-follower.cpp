// overlay-follower.cpp
// A simple Windows C++ program to keep an overlay window always on top and perfectly aligned with a target window (by title)
// Usage: overlay-follower.exe <overlay_window_title> <target_window_title> [offset_y]

// overlay-follower.cpp
#include <windows.h>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: overlay-follower.exe <overlay_window_title> <target_window_title> [offset_y]\n";
        return 1;
    }
    std::string overlayTitle = argv[1];
    std::string targetTitle = argv[2];
    int offsetY = (argc >= 4) ? atoi(argv[3]) : 0;

    HWND overlayHwnd = FindWindowA(NULL, overlayTitle.c_str());
    HWND mpvHwnd = FindWindowA(NULL, targetTitle.c_str());
    HWND electronHwnd = NULL;
    HWND currentTargetHwnd = mpvHwnd;
    std::cout << "[overlay-follower] Started. Overlay: '" << overlayTitle << "', Target: '" << targetTitle << "', OffsetY: " << offsetY << std::endl;

    // Try to find windows for up to 5 seconds
    int retryCount = 0;
    while ((!overlayHwnd || !mpvHwnd) && retryCount < 500) {
        Sleep(10);
        overlayHwnd = FindWindowA(NULL, overlayTitle.c_str());
        mpvHwnd = FindWindowA(NULL, targetTitle.c_str());
        if (retryCount % 50 == 0) {
            std::cout << "[overlay-follower] Waiting for windows... overlayHwnd=" << overlayHwnd << ", mpvHwnd=" << mpvHwnd << std::endl;
        }
        retryCount++;
    }
    if (!overlayHwnd) {
        std::cerr << "[overlay-follower] Could not find overlay window after retry: " << overlayTitle << std::endl;
        return 2;
    }
    if (!mpvHwnd) {
        std::cerr << "[overlay-follower] Could not find target window after retry: " << targetTitle << std::endl;
        return 3;
    }
    std::cout << "[overlay-follower] Windows found. Entering sync loop." << std::endl;

    RECT lastRect = {0};
    int bringToTopCounter = 0;
    int checkEmbedCounter = 0;
    while (IsWindow(overlayHwnd) && IsWindow(mpvHwnd))
    {
        // Every 100ms, check if MPV is embedded in another window
        if (++checkEmbedCounter >= 25) {
            HWND parent = GetParent(mpvHwnd);
            if (parent && parent != electronHwnd) {
                // MPV is embedded, switch to parent (Electron)
                electronHwnd = parent;
                if (currentTargetHwnd != electronHwnd) {
                    std::cout << "[overlay-follower] MPV is embedded, switching tracking to Electron window: " << electronHwnd << std::endl;
                    currentTargetHwnd = electronHwnd;
                }
            } else if (!parent && currentTargetHwnd != mpvHwnd) {
                // MPV is top-level again
                std::cout << "[overlay-follower] MPV is top-level, switching tracking back to MPV window: " << mpvHwnd << std::endl;
                currentTargetHwnd = mpvHwnd;
            }
            checkEmbedCounter = 0;
        }

        RECT rect;
        if (GetWindowRect(currentTargetHwnd, &rect))
        {
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top - offsetY;
            if (memcmp(&rect, &lastRect, sizeof(RECT)) != 0) {
                std::cout << "[overlay-follower] Moving overlay to (" << rect.left << ", " << (rect.top + offsetY) << ") size (" << width << "x" << height << ")" << std::endl;
            }
            SetWindowPos(
                overlayHwnd, HWND_TOPMOST,
                rect.left, rect.top + offsetY,
                width, height,
                SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
            lastRect = rect;
        } else {
            std::cerr << "[overlay-follower] Failed to get target window rect." << std::endl;
        }
        // If the target window is not the foreground window, minimize overlay
        HWND fg = GetForegroundWindow();
        if (fg != currentTargetHwnd && fg != overlayHwnd) {
            ShowWindow(overlayHwnd, SW_MINIMIZE);
        } else {
            ShowWindow(overlayHwnd, SW_RESTORE);
        }
        // Every ~250ms, try to bring overlay to top (without stealing focus)
        if (++bringToTopCounter >= 30)
        {
            std::cout << "[overlay-follower] BringWindowToTop called." << std::endl;
            BringWindowToTop(overlayHwnd);
            bringToTopCounter = 0;
        }
        Sleep(4);
    }
    std::cout << "[overlay-follower] Exiting: one or both windows destroyed." << std::endl;
    return 0;
}