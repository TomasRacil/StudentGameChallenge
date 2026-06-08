#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include "GameWorld.hpp"

class Server {
public:
    Server(unsigned short port);
    void run();

private:
    void handlePackets();
    void sendUpdates();

    sf::UdpSocket m_socket;
    sf::RenderWindow m_window;
    GameWorld m_world;

    std::map<sf::Uint32, sf::IpAddress> m_clientIps;
    std::map<sf::Uint32, unsigned short> m_clientPorts;
    std::map<std::pair<std::string, unsigned short>, sf::Uint32> m_addressToId;
    std::map<sf::Uint32, std::string> m_clientTeams;
    sf::Uint32 m_nextClientId = 1;

    ServerState m_state = ServerState::Lobby;
    float m_roundTimer = 0.0f;
    int m_speedMultiplier = 1;
};
