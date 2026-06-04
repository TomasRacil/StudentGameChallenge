#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include "Protocol.hpp"
#include "Agent.hpp"
#include "Visualizer.hpp"

class Client {
public:
    Client(const std::string& serverIp, unsigned short serverPort, Agent* agent);
    ~Client();

    void run();

private:
    void sendConnect();
    void sendDisconnect();
    void sendAction(Action action);
    void receivePackets();

    sf::UdpSocket m_socket;
    sf::IpAddress m_serverIp;
    unsigned short m_serverPort;
    
    sf::RenderWindow m_window;
    Agent* m_agent;
    Visualizer m_visualizer;

    VisibleState m_lastState;
    bool m_connected;
};
