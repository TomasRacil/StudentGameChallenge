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
    float respawnTimer = 0.0f;
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

    const std::map<std::string, float>& getTeamScores() const { return m_teamScores; }
    void awardExitPoints(const std::string& teamName) { m_teamScores[teamName] += 10.0f; }
    void resetScores() {
        for (auto& pair : m_teamScores) {
            pair.second = 0.0f;
        }
    }

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
    int m_width = MAP_WIDTH; // Odd numbers work best for DFS mazes
    int m_height = MAP_HEIGHT;
    std::vector<bool> m_grid;
    float m_tileSize = TILE_SIZE;

    std::map<std::string, float> m_teamScores;
    std::map<sf::Uint32, sf::Uint32> m_bulletShooter;
};
