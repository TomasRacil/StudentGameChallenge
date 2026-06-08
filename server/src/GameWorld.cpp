#include "GameWorld.hpp"
#include "Math.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

GameWorld::GameWorld() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    generateMaze();
}

void GameWorld::generateMazeDFS(int cx, int cy) {
    m_grid[cx + cy * m_width] = false;

    // Directions: right, down, left, up
    int dx[] = { 2, 0, -2, 0 };
    int dy[] = { 0, 2, 0, -2 };
    
    // Shuffle directions
    int dirs[] = { 0, 1, 2, 3 };
    for (int i = 0; i < 4; ++i) {
        int r = i + std::rand() % (4 - i);
        std::swap(dirs[i], dirs[r]);
    }

    for (int i = 0; i < 4; ++i) {
        int nx = cx + dx[dirs[i]];
        int ny = cy + dy[dirs[i]];

        if (nx > 0 && nx < m_width - 1 && ny > 0 && ny < m_height - 1) {
            if (m_grid[nx + ny * m_width]) { // If wall
                // Knock down wall between
                m_grid[cx + dx[dirs[i]] / 2 + (cy + dy[dirs[i]] / 2) * m_width] = false;
                generateMazeDFS(nx, ny);
            }
        }
    }
}

void GameWorld::generateMaze() {
    // Fill with walls
    m_grid.assign(m_width * m_height, true);

    // Generate maze starting from (1, 1)
    generateMazeDFS(1, 1);

    // Carve out random open rooms
    int numRooms = 5;
    for (int i = 0; i < numRooms; ++i) {
        int roomWidth = 3 + std::rand() % 3; // 3 to 5
        int roomHeight = 3 + std::rand() % 3; // 3 to 5
        if (m_width - 2 - roomWidth < 2 || m_height - 2 - roomHeight < 2) continue;
        int rx = 2 + std::rand() % (m_width - 2 - roomWidth);
        int ry = 2 + std::rand() % (m_height - 2 - roomHeight);
        
        for (int y = ry; y < ry + roomHeight; ++y) {
            for (int x = rx; x < rx + roomWidth; ++x) {
                m_grid[x + y * m_width] = false;
            }
        }
    }

    // Clear top-left corner for safe spawn zone (3x3 open room)
    for (int y = 1; y <= 3; ++y) {
        for (int x = 1; x <= 3; ++x) {
            m_grid[x + y * m_width] = false;
        }
    }

    // Place Exit in bottom-right area
    m_exit.type = EntityType::Exit;
    m_exit.id = 999;
    
    int exitX = m_width - 2;
    int exitY = m_height - 2;
    // ensure exit area is clear
    for (int y = exitY - 1; y <= exitY; ++y) {
        for (int x = exitX - 1; x <= exitX; ++x) {
            m_grid[x + y * m_width] = false;
        }
    }
    m_exit.position = sf::Vector2f(exitX * m_tileSize + m_tileSize / 2, exitY * m_tileSize + m_tileSize / 2);
    m_exit.rotation = 0;

    m_ammoPickups.clear();
    m_bullets.clear();
}

void GameWorld::resetPlayers() {
    for (auto& pair : m_players) {
        respawnPlayer(pair.first);
    }
}

void GameWorld::respawnPlayer(sf::Uint32 id) {
    if (m_players.find(id) != m_players.end()) {
        // Spawn randomly in the 3x3 top-left room (tiles x: 1..3, y: 1..3)
        int rx = 1 + std::rand() % 3;
        int ry = 1 + std::rand() % 3;
        float spawnX = rx * m_tileSize + m_tileSize / 2.0f;
        float spawnY = ry * m_tileSize + m_tileSize / 2.0f;

        float offsetX = static_cast<float>(std::rand() % 20 - 10) * 0.8f; // -8.0f to 8.0f
        float offsetY = static_cast<float>(std::rand() % 20 - 10) * 0.8f;
        
        m_players[id].position = sf::Vector2f(spawnX + offsetX, spawnY + offsetY);
        m_players[id].rotation = 0.0f;
        m_players[id].ammo = 0;
        m_players[id].health = 100.0f;
        m_players[id].currentAction = Action::None;
        m_players[id].respawnTimer = 0.0f;
    }
}

void GameWorld::addPlayer(sf::Uint32 id, const std::string& teamName) {
    PlayerData pd;
    pd.id = id;
    pd.teamName = teamName;
    
    // Spawn randomly in the 3x3 top-left room (tiles x: 1..3, y: 1..3)
    int rx = 1 + std::rand() % 3;
    int ry = 1 + std::rand() % 3;
    float spawnX = rx * m_tileSize + m_tileSize / 2.0f;
    float spawnY = ry * m_tileSize + m_tileSize / 2.0f;

    float offsetX = static_cast<float>(std::rand() % 20 - 10) * 0.8f;
    float offsetY = static_cast<float>(std::rand() % 20 - 10) * 0.8f;
    
    pd.position = sf::Vector2f(spawnX + offsetX, spawnY + offsetY);
    pd.rotation = 0.0f;
    pd.ammo = 0;
    pd.health = 100.0f;
    pd.color = sf::Color(std::rand() % 200 + 55, std::rand() % 200 + 55, std::rand() % 200 + 55);
    pd.currentAction = Action::None;
    
    m_players[id] = pd;
    
    if (m_teamScores.find(teamName) == m_teamScores.end()) {
        m_teamScores[teamName] = 0.0f;
    }
    
    std::cout << "Player " << id << " (" << teamName << ") added.\n";
}

void GameWorld::removePlayer(sf::Uint32 id) {
    m_players.erase(id);
    std::cout << "Player " << id << " removed.\n";
}

void GameWorld::applyAction(sf::Uint32 id, Action action) {
    if (m_players.find(id) != m_players.end()) {
        m_players[id].currentAction = action;
    }
}

bool GameWorld::checkExitReached(sf::Uint32 id) const {
    if (m_players.find(id) == m_players.end()) return false;
    float dist = Math::distance(m_players.at(id).position, m_exit.position);
    return dist < m_tileSize;
}

bool GameWorld::checkCollision(sf::Vector2f pos) const {
    const float R = 10.0f; // Player radius
    int cx = static_cast<int>(pos.x / m_tileSize);
    int cy = static_cast<int>(pos.y / m_tileSize);

    for (int y = cy - 1; y <= cy + 1; ++y) {
        for (int x = cx - 1; x <= cx + 1; ++x) {
            if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
                // Treat out of bounds as collision
                if (pos.x - R < 0 || pos.x + R >= m_width * m_tileSize ||
                    pos.y - R < 0 || pos.y + R >= m_height * m_tileSize) {
                    return true;
                }
                continue;
            }
            if (m_grid[x + y * m_width]) {
                float minX = x * m_tileSize;
                float maxX = (x + 1) * m_tileSize;
                float minY = y * m_tileSize;
                float maxY = (y + 1) * m_tileSize;

                // Find closest point on AABB to circle center
                float closestX = std::max(minX, std::min(pos.x, maxX));
                float closestY = std::max(minY, std::min(pos.y, maxY));

                float dx = pos.x - closestX;
                float dy = pos.y - closestY;
                float distSq = dx * dx + dy * dy;

                if (distSq < R * R) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool GameWorld::hasLineOfSight(sf::Vector2f a, sf::Vector2f b) const {
    float dist = Math::distance(a, b);
    if (dist < 0.1f) return true;
    
    sf::Vector2f dir = (b - a) / dist;
    float step = 5.0f; // Fine step
    
    int targetX = static_cast<int>(b.x / m_tileSize);
    int targetY = static_cast<int>(b.y / m_tileSize);

    for (float d = 0; d < dist; d += step) {
        sf::Vector2f p = a + dir * d;
        int px = static_cast<int>(p.x / m_tileSize);
        int py = static_cast<int>(p.y / m_tileSize);
        
        if (px == targetX && py == targetY) continue; // Target tile itself
        if (px < 0 || px >= m_width || py < 0 || py >= m_height) return false;
        if (m_grid[px + py * m_width]) return false;
    }
    return true;
}

void GameWorld::update(float dt) {
    const float moveSpeed = 150.0f;
    const float rotSpeed = 180.0f;

    // Ensure the number of ammo boxes is exactly 2 * players
    size_t targetAmmoCount = m_players.size();
    while (m_ammoPickups.size() < targetAmmoCount) {
        int rx, ry;
        do {
            rx = 1 + std::rand() % (m_width - 2);
            ry = 1 + std::rand() % (m_height - 2);
        } while (m_grid[rx + ry * m_width]);
        
        Entity ammo;
        ammo.type = EntityType::Ammo;
        ammo.id = m_nextEntityId++;
        ammo.position = sf::Vector2f(rx * m_tileSize + m_tileSize/2, ry * m_tileSize + m_tileSize/2);
        ammo.rotation = 0;
        m_ammoPickups.push_back(ammo);
    }
    while (m_ammoPickups.size() > targetAmmoCount) {
        m_ammoPickups.pop_back();
    }

    for (auto& pair : m_players) {
        PlayerData& p = pair.second;
        if (p.health <= 0) {
            if (p.respawnTimer <= 0.0f) {
                p.respawnTimer = 3.0f; // 3 seconds respawn penalty
                p.position = sf::Vector2f(-1000.0f, -1000.0f); // Hide off-screen
            }
            p.respawnTimer -= dt;
            if (p.respawnTimer <= 0.0f) {
                respawnPlayer(pair.first);
            }
            continue; 
        }

        sf::Vector2f dir = Math::getDirection(p.rotation);

        if (p.currentAction == Action::MoveForward) {
            sf::Vector2f nextPos = p.position + dir * moveSpeed * dt;
            if (!checkCollision(nextPos)) p.position = nextPos;
        } else if (p.currentAction == Action::MoveBackward) {
            sf::Vector2f nextPos = p.position - dir * moveSpeed * dt;
            if (!checkCollision(nextPos)) p.position = nextPos;
        } else if (p.currentAction == Action::RotateLeft) {
            p.rotation -= rotSpeed * dt;
        } else if (p.currentAction == Action::RotateRight) {
            p.rotation += rotSpeed * dt;
        } else if (p.currentAction == Action::PickupAmmo) {
            for (auto it = m_ammoPickups.begin(); it != m_ammoPickups.end(); ) {
                if (Math::distance(p.position, it->position) < m_tileSize) {
                    p.ammo += 5; // Picking up grants 5 ammo
                    it = m_ammoPickups.erase(it);
                } else {
                    ++it;
                }
            }
        } else if (p.currentAction == Action::Shoot) {
            if (p.ammo > 0) {
                p.ammo--;
                Entity bullet;
                bullet.type = EntityType::Bullet;
                bullet.id = m_nextEntityId++;
                // Spawn bullet outside of player radius (10.0f) + a bit of buffer
                bullet.position = p.position + dir * 15.0f;
                bullet.rotation = p.rotation;
                m_bullets.push_back(bullet);
                m_bulletShooter[bullet.id] = p.id;
            }
        }
        
        // Reset action
        p.currentAction = Action::None;
    }

    // Resolve player-player overlap
    for (auto& pairA : m_players) {
        for (auto& pairB : m_players) {
            if (pairA.first != pairB.first) {
                sf::Vector2f posA = pairA.second.position;
                sf::Vector2f posB = pairB.second.position;
                float dist = Math::distance(posA, posB);
                if (dist < 22.0f) { // Soft distancing buffer (slightly larger than 2 * player_radius)
                    float overlap = 22.0f - dist;
                    sf::Vector2f pushDir;
                    if (dist > 0.1f) {
                        pushDir = (posA - posB) / dist;
                    } else {
                        // If perfectly overlapping, push in random direction
                        float angle = static_cast<float>(std::rand() % 360) * 3.14159f / 180.0f;
                        pushDir = sf::Vector2f(std::cos(angle), std::sin(angle));
                    }

                    // Try to push A and B by half the overlap each
                    sf::Vector2f pushHalfA = pushDir * (overlap * 0.5f);
                    sf::Vector2f pushHalfB = -pushDir * (overlap * 0.5f);

                    bool movedA = false;
                    bool movedB = false;

                    if (!checkCollision(posA + pushHalfA)) {
                        pairA.second.position = posA + pushHalfA;
                        movedA = true;
                    }
                    if (!checkCollision(posB + pushHalfB)) {
                        pairB.second.position = posB + pushHalfB;
                        movedB = true;
                    }

                    // If A couldn't move (blocked by wall), B takes the full push to separate
                    if (!movedA && movedB) {
                        sf::Vector2f pushFullB = -pushDir * overlap;
                        if (!checkCollision(posB + pushFullB)) {
                            pairB.second.position = posB + pushFullB;
                        }
                    }
                    // If B couldn't move (blocked by wall), A takes the full push to separate
                    if (!movedB && movedA) {
                        sf::Vector2f pushFullA = pushDir * overlap;
                        if (!checkCollision(posA + pushFullA)) {
                            pairA.second.position = posA + pushFullA;
                        }
                    }
                }
            }
        }
    }

    // Update bullets
    for (auto it = m_bullets.begin(); it != m_bullets.end(); ) {
        sf::Vector2f dir = Math::getDirection(it->rotation);
        // Faster bullets: 800.0f
        it->position += dir * 800.0f * dt;

        if (checkCollision(it->position)) {
            it = m_bullets.erase(it);
            continue;
        }

        bool hit = false;
        sf::Uint32 shooterId = 0;
        sf::Uint32 victimId = 0;

        for (auto& pair : m_players) {
            if (Math::distance(it->position, pair.second.position) < 15.0f) {
                pair.second.health -= 50.0f; // Survive only one bullet, second will kill them
                hit = true;
                victimId = pair.first;

                auto shooterIt = m_bulletShooter.find(it->id);
                if (shooterIt != m_bulletShooter.end()) {
                    shooterId = shooterIt->second;
                }
                break;
            }
        }

        if (hit) {
            // Pokud oběť zemřela, upravíme skóre
            if (victimId != 0 && m_players[victimId].health <= 0) {
                // Oběť ztratí 2 body
                std::string victimTeam = m_players[victimId].teamName;
                m_teamScores[victimTeam] -= 2.0f;

                // Střelec získá 2 body (pokud to nebyla sebevražda)
                if (shooterId != 0 && m_players.find(shooterId) != m_players.end() && shooterId != victimId) {
                    std::string shooterTeam = m_players[shooterId].teamName;
                    m_teamScores[shooterTeam] += 2.0f;
                }
            }
            m_bulletShooter.erase(it->id);
            it = m_bullets.erase(it);
        } else {
            ++it;
        }
    }
}

VisibleState GameWorld::getVisibleStateFor(sf::Uint32 id) const {
    VisibleState state;
    if (m_players.find(id) == m_players.end()) return state;
    
    const PlayerData& p = m_players.at(id);
    state.myAmmo = p.ammo;
    state.myHealth = p.health;
    state.myPosition = p.position;
    state.myRotation = p.rotation;

    // Visible walls (checking all walls in the grid since vision has no range limit)
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (m_grid[x + y * m_width]) {
                sf::Vector2f center(x * m_tileSize + m_tileSize/2, y * m_tileSize + m_tileSize/2);
                if (hasLineOfSight(p.position, center)) {
                    Entity wall;
                    wall.type = EntityType::Wall;
                    wall.id = 0;
                    wall.position = center;
                    wall.rotation = 0;
                    state.entities.push_back(wall);
                }
            }
        }
    }

    // Visible other players
    for (const auto& pair : m_players) {
        if (pair.first != id && hasLineOfSight(p.position, pair.second.position)) {
            Entity other;
            other.type = EntityType::Player;
            other.id = pair.first;
            other.position = pair.second.position;
            other.rotation = pair.second.rotation;
            state.entities.push_back(other);
        }
    }

    // Visible Ammo
    for (const auto& a : m_ammoPickups) {
        if (hasLineOfSight(p.position, a.position)) {
            state.entities.push_back(a);
        }
    }

    // Visible bullets
    for (const auto& b : m_bullets) {
        if (hasLineOfSight(p.position, b.position)) {
            state.entities.push_back(b);
        }
    }

    // Visible Exit
    if (hasLineOfSight(p.position, m_exit.position)) {
        state.entities.push_back(m_exit);
    }

    return state;
}

void GameWorld::renderTeacherView(sf::RenderTarget& target) const {
    // Clear to dark navy blue/black
    target.clear(sf::Color(10, 12, 18));

    // Animation time
    static sf::Clock animClock;
    float time = animClock.getElapsedTime().asSeconds();

    // Font loading for player names
    sf::Font font;
    bool hasFont = font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    // Draw walls
    sf::RectangleShape wall(sf::Vector2f(m_tileSize - 2.0f, m_tileSize - 2.0f));
    wall.setOrigin(m_tileSize / 2.0f - 1.0f, m_tileSize / 2.0f - 1.0f);
    wall.setFillColor(sf::Color(15, 20, 30));
    wall.setOutlineThickness(1.0f);
    wall.setOutlineColor(sf::Color(0, 130, 220));

    sf::RectangleShape innerPlate(sf::Vector2f(m_tileSize - 12.0f, m_tileSize - 12.0f));
    innerPlate.setOrigin(m_tileSize / 2.0f - 6.0f, m_tileSize / 2.0f - 6.0f);
    innerPlate.setFillColor(sf::Color(24, 30, 42));
    innerPlate.setOutlineThickness(0.5f);
    innerPlate.setOutlineColor(sf::Color(0, 70, 130));

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (m_grid[x + y * m_width]) {
                float px = x * m_tileSize + m_tileSize / 2.0f;
                float py = y * m_tileSize + m_tileSize / 2.0f;

                wall.setPosition(px, py);
                target.draw(wall);

                innerPlate.setPosition(px, py);
                target.draw(innerPlate);
            }
        }
    }

    // Exit portal (Green glow & rotating frames)
    sf::CircleShape exitGlow(m_tileSize / 1.6f + 2.0f * std::sin(time * 4.0f));
    exitGlow.setOrigin(exitGlow.getRadius(), exitGlow.getRadius());
    exitGlow.setPosition(m_exit.position);
    exitGlow.setFillColor(sf::Color(0, 255, 100, 35));
    target.draw(exitGlow);

    sf::RectangleShape exitFrame(sf::Vector2f(m_tileSize - 6.0f, m_tileSize - 6.0f));
    exitFrame.setOrigin(m_tileSize / 2.0f - 3.0f, m_tileSize / 2.0f - 3.0f);
    exitFrame.setPosition(m_exit.position);
    exitFrame.setFillColor(sf::Color::Transparent);
    exitFrame.setOutlineThickness(1.5f);
    exitFrame.setOutlineColor(sf::Color(0, 255, 128));
    exitFrame.setRotation(time * 45.0f);
    target.draw(exitFrame);

    sf::CircleShape exitCore(6.0f);
    exitCore.setOrigin(6.0f, 6.0f);
    exitCore.setPosition(m_exit.position);
    exitCore.setFillColor(sf::Color(255, 255, 255, 200));
    target.draw(exitCore);

    // Ammo Pickups (Yellow crystals)
    for (const auto& a : m_ammoPickups) {
        float pulse = 1.0f + 0.15f * std::sin(time * 6.0f);
        
        sf::CircleShape aura(7.0f * pulse);
        aura.setOrigin(7.0f * pulse, 7.0f * pulse);
        aura.setPosition(a.position);
        aura.setFillColor(sf::Color(255, 215, 0, 50));
        target.draw(aura);

        sf::RectangleShape crystal(sf::Vector2f(7.0f * pulse, 7.0f * pulse));
        crystal.setOrigin(3.5f * pulse, 3.5f * pulse);
        crystal.setPosition(a.position);
        crystal.setFillColor(sf::Color(255, 230, 0));
        crystal.setOutlineThickness(0.8f);
        crystal.setOutlineColor(sf::Color::White);
        crystal.setRotation(time * 90.0f);
        target.draw(crystal);
    }

    // Bullets (Streaks)
    for (const auto& b : m_bullets) {
        sf::CircleShape head(2.5f);
        head.setOrigin(2.5f, 2.5f);
        head.setPosition(b.position);
        head.setFillColor(sf::Color(255, 255, 200));
        target.draw(head);

        float rad = b.rotation * 3.14159f / 180.0f;
        sf::Vector2f dir(std::cos(rad), std::sin(rad));
        sf::Vertex trail[] = {
            sf::Vertex(b.position, sf::Color(255, 80, 0, 255)),
            sf::Vertex(b.position - dir * 15.0f, sf::Color(255, 0, 0, 0))
        };
        target.draw(trail, 2, sf::Lines);
    }

    // Players (Sleek starships with their team colors & names)
    for (const auto& pair : m_players) {
        const PlayerData& p = pair.second;
        if (p.health <= 0) continue;

        // Bottom glow of player's team color
        sf::CircleShape glow(18.0f);
        glow.setOrigin(18.0f, 18.0f);
        glow.setPosition(p.position);
        sf::Color playerColor = p.color;
        glow.setFillColor(sf::Color(playerColor.r, playerColor.g, playerColor.b, 40));
        target.draw(glow);

        // Player ship shape
        sf::ConvexShape ship;
        ship.setPointCount(4);
        ship.setPoint(0, sf::Vector2f(14.0f, 0.0f));
        ship.setPoint(1, sf::Vector2f(-10.0f, -8.0f));
        ship.setPoint(2, sf::Vector2f(-5.0f, 0.0f));
        ship.setPoint(3, sf::Vector2f(-10.0f, 8.0f));
        ship.setOrigin(0.0f, 0.0f);
        ship.setPosition(p.position);
        ship.setRotation(p.rotation);
        ship.setFillColor(playerColor);
        ship.setOutlineThickness(1.5f);
        ship.setOutlineColor(sf::Color::White);
        target.draw(ship);

        // Direction line
        sf::Vector2f dir = Math::getDirection(p.rotation);
        sf::Vertex line[] = {
            sf::Vertex(p.position + dir * 14.0f, sf::Color(playerColor.r, playerColor.g, playerColor.b, 200)),
            sf::Vertex(p.position + dir * 32.0f, sf::Color(playerColor.r, playerColor.g, playerColor.b, 0))
        };
        target.draw(line, 2, sf::Lines);

        // Team name text label above player
        if (hasFont) {
            sf::Text nameText;
            nameText.setFont(font);
            nameText.setString(p.teamName);
            nameText.setCharacterSize(12);
            nameText.setStyle(sf::Text::Bold);
            nameText.setFillColor(playerColor);
            
            // Draw a subtle dark shadow behind text for legibility
            sf::Text shadow = nameText;
            shadow.setFillColor(sf::Color(0, 0, 0, 200));
            
            sf::FloatRect bounds = nameText.getLocalBounds();
            nameText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            shadow.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            
            nameText.setPosition(p.position.x, p.position.y - 24.0f);
            shadow.setPosition(p.position.x + 1.0f, p.position.y - 23.0f);
            
            target.draw(shadow);
            target.draw(nameText);
        }
    }

    // Render Leaderboard on Teacher View (Server)
    sf::View originalView = target.getView();
    target.setView(target.getDefaultView());

    if (hasFont) {
        sf::RectangleShape panel(sf::Vector2f(240.0f, 220.0f));
        panel.setPosition(540.0f, 20.0f);
        panel.setFillColor(sf::Color(12, 15, 23, 210));
        panel.setOutlineThickness(1.0f);
        panel.setOutlineColor(sf::Color(0, 150, 255, 100));
        target.draw(panel);

        sf::Vertex techLines[] = {
            sf::Vertex(sf::Vector2f(540.0f, 55.0f), sf::Color(0, 150, 255, 100)),
            sf::Vertex(sf::Vector2f(780.0f, 55.0f), sf::Color(0, 150, 255, 100))
        };
        target.draw(techLines, 2, sf::Lines);

        sf::Text panelHeader;
        panelHeader.setFont(font);
        panelHeader.setString("LEADERBOARD");
        panelHeader.setCharacterSize(14);
        panelHeader.setStyle(sf::Text::Bold);
        panelHeader.setFillColor(sf::Color(0, 180, 255));
        panelHeader.setPosition(555.0f, 28.0f);
        target.draw(panelHeader);

        // Sort scores descending
        std::vector<std::pair<std::string, float>> sortedScores(m_teamScores.begin(), m_teamScores.end());
        std::sort(sortedScores.begin(), sortedScores.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        float scoreY = 65.0f;
        for (const auto& score : sortedScores) {
            if (scoreY > 215.0f) break;
            
            sf::Text scoreText;
            scoreText.setFont(font);
            scoreText.setCharacterSize(12);
            scoreText.setFillColor(sf::Color(200, 220, 245));
            scoreText.setPosition(555.0f, scoreY);

            std::string scoreStr = score.first + ": " + std::to_string(static_cast<int>(score.second)) + " pts";
            scoreText.setString(scoreStr);
            target.draw(scoreText);
            scoreY += 20.0f;
        }
    }

    target.setView(originalView);
}
