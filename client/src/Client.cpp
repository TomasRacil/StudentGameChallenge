#include "Client.hpp"
#include <iostream>

Client::Client(const std::string& serverIp, unsigned short serverPort, Agent* agent)
    : m_serverIp(serverIp), m_serverPort(serverPort), m_agent(agent), m_connected(false)
{
    m_socket.setBlocking(false);
}

Client::~Client() {
    sendDisconnect();
}

void Client::sendConnect() {
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(PacketType::Connect);
    packet << m_agent->getTeamName();
    m_socket.send(packet, m_serverIp, m_serverPort);
}

void Client::sendDisconnect() {
    if (m_connected) {
        sf::Packet packet;
        packet << static_cast<sf::Uint8>(PacketType::Disconnect);
        m_socket.send(packet, m_serverIp, m_serverPort);
    }
}

void Client::sendAction(Action action) {
    if (action != Action::None) {
        sf::Packet packet;
        packet << static_cast<sf::Uint8>(PacketType::Input);
        packet << static_cast<sf::Uint8>(action);
        m_socket.send(packet, m_serverIp, m_serverPort);
    }
}

void Client::receivePackets() {
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short port;

    while (m_socket.receive(packet, sender, port) == sf::Socket::Done) {
        if (sender != m_serverIp || port != m_serverPort) continue;

        sf::Uint8 typeByte;
        if (!(packet >> typeByte)) continue;
        PacketType type = static_cast<PacketType>(typeByte);

        if (type == PacketType::Welcome) {
            sf::Uint32 myId;
            packet >> myId;
            m_connected = true;
            std::cout << "Connected to server! My ID: " << myId << std::endl;
        } else if (type == PacketType::GameStateUpdate) {
            VisibleState state;
            
            sf::Uint8 sState;
            packet >> sState >> state.currentRoundTime;
            state.serverState = static_cast<ServerState>(sState);

            sf::Uint32 scoreCount;
            packet >> scoreCount;
            for (sf::Uint32 i = 0; i < scoreCount; ++i) {
                TeamScore ts;
                packet >> ts.teamName >> ts.bestTime;
                state.scoreboard.push_back(ts);
            }

            packet >> state.myAmmo >> state.myHealth >> state.myPosition.x >> state.myPosition.y >> state.myRotation;
            
            sf::Uint32 entityCount;
            packet >> entityCount;
            for (sf::Uint32 i = 0; i < entityCount; ++i) {
                Entity e;
                packet >> e;
                state.entities.push_back(e);
            }
            m_lastState = state;
        }
    }
}

void Client::run() {
    sendConnect();

    sf::Clock clock;
    const float TimePerFrame = 1.0f / 60.0f;
    float timeSinceLastUpdate = 0.0f;
    bool running = true;

    while (running) {
        if (m_window.isOpen()) {
            sf::Event event;
            while (m_window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    m_window.close();
                    running = false;
                }
            }
        }

        receivePackets();

        float dt = clock.restart().asSeconds();
        timeSinceLastUpdate += dt;

        while (timeSinceLastUpdate > TimePerFrame) {
            timeSinceLastUpdate -= TimePerFrame;
            
            // Allow agent to update and make decisions
            if (m_connected) {
                m_agent->update(m_lastState);
                if (m_lastState.serverState == ServerState::Playing) {
                    Action action = m_agent->getAction();
                    sendAction(action);
                }
            }
        }

        // Open the window once the match starts for the first time
        if (m_connected && m_lastState.serverState == ServerState::Playing && !m_window.isOpen()) {
            m_window.create(sf::VideoMode(800, 800), "Student View - Client");
        }

        // If the window is open, keep rendering and updating it
        if (m_window.isOpen()) {
            m_visualizer.render(m_window, m_lastState);
            m_window.display();
        } else {
            sf::sleep(sf::milliseconds(16));
        }
    }
}
