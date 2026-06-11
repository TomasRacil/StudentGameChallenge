#ifdef _WIN32
#include <windows.h>
#endif

#include "Client.hpp"
#include <iostream>

// Factory function implemented in MyAgent.cpp
extern Agent* createStudentAgent(const std::string& teamName);

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Disable QuickEdit mode to prevent console click hangs
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;
    if (GetConsoleMode(hInput, &prevMode)) {
        SetConsoleMode(hInput, prevMode & ~ENABLE_QUICK_EDIT_MODE);
    }
#endif

    std::string teamName;
    std::string serverIp = "127.0.0.1";

    if (argc > 1) {
        teamName = argv[1];
        std::cout << "Team Name provided via argument: " << teamName << "\n";
        if (argc > 2) {
            serverIp = argv[2];
            std::cout << "Server IP provided via argument: " << serverIp << "\n";
        }
    } else {
        std::cout << "Welcome to the Game Challenge!\n";
        std::cout << "Enter your Team Name: ";
        std::getline(std::cin, teamName);

        std::cout << "Enter Server IP [default: 127.0.0.1]: ";
        std::string inputIp;
        std::getline(std::cin, inputIp);
        if (!inputIp.empty()) {
            serverIp = inputIp;
        }
    }

    Agent* agent = createStudentAgent(teamName);
    Client client(serverIp, 54321, agent);
    client.run();
    delete agent;
    return 0;
}
