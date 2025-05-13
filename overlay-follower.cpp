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
    HWND targetHwnd = FindWindowA(NULL, targetTitle.c_str());
    std::cout << "[overlay-follower] Started. Overlay: '" << overlayTitle << "', Target: '" << targetTitle << "', OffsetY: " << offsetY << std::endl;

    // Try to find windows for up to 5 seconds
    int retryCount = 0;
    while ((!overlayHwnd || !targetHwnd) && retryCount < 500) {
        Sleep(10);
        overlayHwnd = FindWindowA(NULL, overlayTitle.c_str());
        targetHwnd = FindWindowA(NULL, targetTitle.c_str());
        if (retryCount % 50 == 0) {
            std::cout << "[overlay-follower] Waiting for windows... overlayHwnd=" << overlayHwnd << ", targetHwnd=" << targetHwnd << std::endl;
        }
        retryCount++;
    }
    if (!overlayHwnd) {
        std::cerr << "[overlay-follower] Could not find overlay window after retry: " << overlayTitle << std::endl;
        return 2;
    }
    if (!targetHwnd) {
        std::cerr << "[overlay-follower] Could not find target window after retry: " << targetTitle << std::endl;
        return 3;
    }
    std::cout << "[overlay-follower] Windows found. Entering sync loop." << std::endl;

    RECT lastRect = {0};
    int bringToTopCounter = 0;
    while (IsWindow(overlayHwnd) && IsWindow(targetHwnd))
    {
        RECT rect;
        if (GetWindowRect(targetHwnd, &rect))
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
                SWP_SHOWWINDOW | SWP_NOACTIVATE);
            lastRect = rect;
        } else {
            std::cerr << "[overlay-follower] Failed to get target window rect." << std::endl;
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