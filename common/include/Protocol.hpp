#pragma once
#include <SFML/Network.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <cstdint>
#include <string>

const int MAP_WIDTH = 15;
const int MAP_HEIGHT = 15;
const float TILE_SIZE = 80.0f;

enum class PacketType : sf::Uint8 {
    // Client -> Server
    Connect = 0,
    Input,
    Disconnect,

    // Server -> Client
    Welcome,
    GameStateUpdate
};

enum class ServerState : sf::Uint8 {
    Lobby = 0,
    Playing,
    RoundEnd
};

enum class Action : sf::Uint8 {
    None = 0,
    MoveForward,
    MoveBackward,
    RotateLeft,
    RotateRight,
    Shoot,
    PickupAmmo
};

enum class EntityType : sf::Uint8 {
    Player = 0,
    Wall,
    Ammo,
    Bullet,
    Exit
};

struct Entity {
    EntityType type;
    sf::Uint32 id;
    sf::Vector2f position;
    float rotation; // Degrees
};

inline sf::Packet& operator<<(sf::Packet& packet, const Entity& entity) {
    return packet << static_cast<sf::Uint8>(entity.type) << entity.id << entity.position.x << entity.position.y << entity.rotation;
}

inline sf::Packet& operator>>(sf::Packet& packet, Entity& entity) {
    sf::Uint8 type;
    packet >> type >> entity.id >> entity.position.x >> entity.position.y >> entity.rotation;
    entity.type = static_cast<EntityType>(type);
    return packet;
}

struct TeamScore {
    std::string teamName;
    float score; // Points accumulated.
};

struct VisibleState {
    ServerState serverState;
    float currentRoundTime;
    std::vector<TeamScore> scoreboard;

    std::vector<Entity> entities;
    int myAmmo;
    float myHealth;
    sf::Vector2f myPosition;
    float myRotation;
};
