#ifdef _WIN32
#include <windows.h>
#endif
#include "Server.hpp"

int main() {
#ifdef _WIN32
    // Disable QuickEdit mode to prevent console click hangs
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;
    if (GetConsoleMode(hInput, &prevMode)) {
        SetConsoleMode(hInput, prevMode & ~ENABLE_QUICK_EDIT_MODE);
    }
#endif

    Server server(54321);
    server.run();
    return 0;
}
