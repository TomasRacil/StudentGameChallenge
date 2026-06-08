#include "Server.hpp"
#include <iostream>
#include <SFML/Window/Keyboard.hpp>

Server::Server(unsigned short port) : m_window(sf::VideoMode(800, 800), "Teacher View - Server") {
    if (m_socket.bind(port) != sf::Socket::Done) {
        std::cerr << "Failed to bind server to port " << port << std::endl;
    } else {
        std::cout << "Server listening on port " << port << std::endl;
    }
    m_socket.setBlocking(false);
    
    // Scale the view to fit the maze into the 800x800 window
    m_window.setView(sf::View(sf::FloatRect(0.0f, 0.0f, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE)));
}

void Server::run() {
    sf::Clock clock;
    float timeSinceLastUpdate = 0.0f;
    const float TimePerFrame = 1.0f / 60.0f; // 60 updates per second
    const float NetworkTickRate = 1.0f / 20.0f; // send state 20 times per second
    float timeSinceLastNetworkUpdate = 0.0f;

    while (m_window.isOpen()) {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::R) {
                    std::cout << "Restarting round (scores preserved, new maze generated)..." << std::endl;
                    m_world.generateMaze();
                    m_world.resetPlayers();
                    m_state = ServerState::Lobby;
                    m_roundTimer = 0.0f;
                }
            }
        }

        // Handle speed multiplier keys (1-4)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1)) {
            m_speedMultiplier = 1;
            m_window.setTitle("Teacher View - Server (Speed: 1x)");
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2)) {
            m_speedMultiplier = 2;
            m_window.setTitle("Teacher View - Server (Speed: 2x)");
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad3)) {
            m_speedMultiplier = 4;
            m_window.setTitle("Teacher View - Server (Speed: 4x)");
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4) || sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad4)) {
            m_speedMultiplier = 8;
            m_window.setTitle("Teacher View - Server (Speed: 8x)");
        }

        handlePackets();

        float dt = clock.restart().asSeconds();
        timeSinceLastUpdate += dt;
        timeSinceLastNetworkUpdate += dt;

        // State machine logic
        if (m_state == ServerState::Lobby) {
            static sf::Clock lobbyClock;
            bool hasPlayers = !m_clientTeams.empty();
            if (!hasPlayers) {
                lobbyClock.restart();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) || (hasPlayers && lobbyClock.getElapsedTime().asSeconds() > 3.0f)) {
                std::cout << "Starting Match!" << std::endl;
                m_world.generateMaze();
                m_world.resetPlayers();
                m_state = ServerState::Playing;
                m_roundTimer = 0.0f;
            }
        } else if (m_state == ServerState::RoundEnd) {
            m_roundTimer -= dt;
            if (m_roundTimer <= 0.0f) {
                m_state = ServerState::Lobby;
            }
        }

        while (timeSinceLastUpdate > TimePerFrame) {
            timeSinceLastUpdate -= TimePerFrame;
            if (m_state == ServerState::Playing) {
                for (int i = 0; i < m_speedMultiplier; ++i) {
                    m_world.update(TimePerFrame);
                    m_roundTimer += TimePerFrame;

                    // Check if anyone reached the exit
                    sf::Uint32 winnerId = 0;
                    for (const auto& pair : m_clientTeams) {
                        if (m_world.checkExitReached(pair.first)) {
                            winnerId = pair.first;
                            break;
                        }
                    }

                    if (winnerId != 0) {
                        std::string teamName = m_clientTeams[winnerId];
                        std::cout << "Team " << teamName << " reached the exit in " << m_roundTimer << " seconds!\n";
                        
                        // Update score: reaching the exit is for 10 points
                        m_world.awardExitPoints(teamName);

                        m_state = ServerState::RoundEnd;
                        m_roundTimer = 5.0f; // Wait 5 seconds before going to lobby
                        break; // Exit the speedMultiplier loop
                    }
                }
            }
        }

        if (timeSinceLastNetworkUpdate > NetworkTickRate) {
            timeSinceLastNetworkUpdate -= NetworkTickRate;
            sendUpdates();
        }

        m_world.renderTeacherView(m_window);
        m_window.display();
    }
}

void Server::handlePackets() {
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short port;

    while (m_socket.receive(packet, sender, port) == sf::Socket::Done) {
        sf::Uint8 typeByte;
        if (!(packet >> typeByte)) continue;

        PacketType type = static_cast<PacketType>(typeByte);
        auto clientKey = std::make_pair(sender.toString(), port);
        
        if (type == PacketType::Connect) {
            std::string teamName;
            if (!(packet >> teamName)) continue;

            if (m_addressToId.find(clientKey) == m_addressToId.end()) {
                sf::Uint32 id = m_nextClientId++;
                m_addressToId[clientKey] = id;
                m_clientIps[id] = sender;
                m_clientPorts[id] = port;
                m_clientTeams[id] = teamName;
                
                m_world.addPlayer(id, teamName);

                // Send welcome packet
                sf::Packet outPacket;
                outPacket << static_cast<sf::Uint8>(PacketType::Welcome) << id;
                m_socket.send(outPacket, sender, port);
            }
        } else {
            // Needs to be an existing client
            if (m_addressToId.find(clientKey) != m_addressToId.end()) {
                sf::Uint32 id = m_addressToId[clientKey];

                if (type == PacketType::Input && m_state == ServerState::Playing) {
                    sf::Uint8 actionByte;
                    if (packet >> actionByte) {
                        Action action = static_cast<Action>(actionByte);
                        m_world.applyAction(id, action);
                    }
                } else if (type == PacketType::Disconnect) {
                    m_world.removePlayer(id);
                    m_clientIps.erase(id);
                    m_clientPorts.erase(id);
                    m_clientTeams.erase(id);
                    m_addressToId.erase(clientKey);
                    std::cout << "Client disconnected: " << id << std::endl;
                }
            }
        }
    }
}

void Server::sendUpdates() {
    for (const auto& pair : m_clientIps) {
        sf::Uint32 id = pair.first;
        sf::IpAddress ip = pair.second;
        unsigned short port = m_clientPorts[id];

        VisibleState state = m_world.getVisibleStateFor(id);
        
        sf::Packet packet;
        packet << static_cast<sf::Uint8>(PacketType::GameStateUpdate);

        packet << static_cast<sf::Uint8>(m_state);
        packet << m_roundTimer;
        
        const auto& teamScores = m_world.getTeamScores();
        packet << static_cast<sf::Uint32>(teamScores.size());
        for (const auto& tp : teamScores) {
            packet << tp.first << tp.second;
        }

        packet << state.myAmmo << state.myHealth << state.myPosition.x << state.myPosition.y << state.myRotation;
        
        packet << static_cast<sf::Uint32>(state.entities.size());
        for (const auto& e : state.entities) {
            packet << e;
        }

        m_socket.send(packet, ip, port);
    }
}
