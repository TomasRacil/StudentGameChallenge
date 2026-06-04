#ifdef _WIN32
#include <windows.h>
#endif

#include "Client.hpp"
#include <iostream>

// Factory function implemented in MyAgent.cpp
extern Agent* createStudentAgent(const std::string& teamName);

int main() {
#ifdef _WIN32
    // Disable QuickEdit mode to prevent console click hangs
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;
    if (GetConsoleMode(hInput, &prevMode)) {
        SetConsoleMode(hInput, prevMode & ~ENABLE_QUICK_EDIT_MODE);
    }
#endif

    std::string teamName;
    std::cout << "Welcome to the Game Challenge!\n";
    std::cout << "Enter your Team Name: ";
    std::cin >> teamName;

    Agent* agent = createStudentAgent(teamName);
    Client client("127.0.0.1", 54321, agent);
    client.run();
    delete agent;
    return 0;
}
