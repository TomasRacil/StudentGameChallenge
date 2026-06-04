#pragma once
#include <vector>
#include <map>
#include <SFML/Graphics.hpp>
#include "Protocol.hpp"

struct PlayerData {
    sf::Uint32 id;
    std::string teamName;
    sf::Vector2f position;
    float rotation; // Degrees
    int ammo;
    float health;
    sf::Color color;
    Action currentAction;
};

class GameWorld {
public:
    GameWorld();
    
    void update(float dt);
    void addPlayer(sf::Uint32 id, const std::string& teamName);
    void removePlayer(sf::Uint32 id);
    void applyAction(sf::Uint32 id, Action action);
    
    void generateMaze();
    void resetPlayers();
    bool checkExitReached(sf::Uint32 id) const;
    void respawnPlayer(sf::Uint32 id);

    VisibleState getVisibleStateFor(sf::Uint32 id) const;
    void renderTeacherView(sf::RenderTarget& target) const;

private:
    void generateMazeDFS(int cx, int cy);
    bool checkCollision(sf::Vector2f pos) const;
    bool hasLineOfSight(sf::Vector2f a, sf::Vector2f b) const;

    std::map<sf::Uint32, PlayerData> m_players;
    std::vector<Entity> m_ammoPickups;
    std::vector<Entity> m_bullets;
    Entity m_exit;
    
    sf::Uint32 m_nextEntityId = 1000;

    // Map grid (true = wall, false = empty)
    int m_width = 31; // Odd numbers work best for DFS mazes
    int m_height = 31;
    std::vector<bool> m_grid;
    float m_tileSize = 40.0f;
};
