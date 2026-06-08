#include "Visualizer.hpp"
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>

Visualizer::Visualizer() {
    if (m_font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        m_hasFont = true;
    } else {
        m_hasFont = false;
    }
}

void Visualizer::render(sf::RenderTarget& target, const VisibleState& state) {
    // Clear background to a very dark sci-fi navy blue/black
    target.clear(sf::Color(10, 12, 18));

    sf::View worldView = target.getDefaultView();
    worldView.setCenter(state.myPosition);
    target.setView(worldView);

    // Get time for animations
    static sf::Clock animClock;
    float time = animClock.getElapsedTime().asSeconds();

    // Draw background grid lines
    float startX = std::floor((state.myPosition.x - 400.0f) / TILE_SIZE) * TILE_SIZE;
    float endX = startX + 800.0f + TILE_SIZE;
    float startY = std::floor((state.myPosition.y - 400.0f) / TILE_SIZE) * TILE_SIZE;
    float endY = startY + 800.0f + TILE_SIZE;

    sf::VertexArray gridLines(sf::Lines);
    for (float x = startX; x <= endX; x += TILE_SIZE) {
        gridLines.append(sf::Vertex(sf::Vector2f(x, startY), sf::Color(25, 35, 55, 90)));
        gridLines.append(sf::Vertex(sf::Vector2f(x, endY), sf::Color(25, 35, 55, 90)));
    }
    for (float y = startY; y <= endY; y += TILE_SIZE) {
        gridLines.append(sf::Vertex(sf::Vector2f(startX, y), sf::Color(25, 35, 55, 90)));
        gridLines.append(sf::Vertex(sf::Vector2f(endX, y), sf::Color(25, 35, 55, 90)));
    }
    target.draw(gridLines);

    // Draw entities
    for (const auto& e : state.entities) {
        if (e.type == EntityType::Wall) {
            // Draw a high-tech beveled wall
            sf::RectangleShape wall(sf::Vector2f(TILE_SIZE - 2.0f, TILE_SIZE - 2.0f));
            wall.setOrigin((TILE_SIZE - 2.0f) / 2.0f, (TILE_SIZE - 2.0f) / 2.0f);
            wall.setPosition(e.position);
            wall.setFillColor(sf::Color(15, 20, 30));
            wall.setOutlineThickness(1.5f);
            wall.setOutlineColor(sf::Color(0, 130, 220));
            target.draw(wall);

            sf::RectangleShape innerPlate(sf::Vector2f(TILE_SIZE - 12.0f, TILE_SIZE - 12.0f));
            innerPlate.setOrigin((TILE_SIZE - 12.0f) / 2.0f, (TILE_SIZE - 12.0f) / 2.0f);
            innerPlate.setPosition(e.position);
            innerPlate.setFillColor(sf::Color(24, 30, 42));
            innerPlate.setOutlineThickness(1.0f);
            innerPlate.setOutlineColor(sf::Color(0, 70, 130));
            target.draw(innerPlate);

            // Tech lines
            sf::RectangleShape line(sf::Vector2f(TILE_SIZE * 0.4f, 2.0f));
            line.setOrigin(TILE_SIZE * 0.2f, 1.0f);
            line.setPosition(e.position);
            line.setFillColor(sf::Color(0, 100, 180, 100));
            target.draw(line);
        } else if (e.type == EntityType::Player) {
            // Draw enemy player ship
            // Bottom red glow
            sf::CircleShape glow(18.0f);
            glow.setOrigin(18.0f, 18.0f);
            glow.setPosition(e.position);
            glow.setFillColor(sf::Color(255, 60, 60, 40));
            target.draw(glow);

            sf::ConvexShape ship;
            ship.setPointCount(4);
            ship.setPoint(0, sf::Vector2f(14.0f, 0.0f));
            ship.setPoint(1, sf::Vector2f(-10.0f, -8.0f));
            ship.setPoint(2, sf::Vector2f(-5.0f, 0.0f));
            ship.setPoint(3, sf::Vector2f(-10.0f, 8.0f));
            ship.setOrigin(0.0f, 0.0f);
            ship.setPosition(e.position);
            ship.setRotation(e.rotation);
            ship.setFillColor(sf::Color(255, 60, 60));
            ship.setOutlineThickness(1.5f);
            ship.setOutlineColor(sf::Color(255, 180, 180));
            target.draw(ship);

            // Enemy label
            if (m_hasFont) {
                sf::Text text;
                text.setFont(m_font);
                text.setString("BOT " + std::to_string(e.id));
                text.setCharacterSize(10);
                text.setFillColor(sf::Color(255, 120, 120));
                sf::FloatRect bounds = text.getLocalBounds();
                text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                text.setPosition(e.position.x, e.position.y - 22.0f);
                target.draw(text);
            }
        } else if (e.type == EntityType::Ammo) {
            // Spinning glowing energy cell
            float pulse = 1.0f + 0.15f * std::sin(time * 6.0f);
            
            sf::CircleShape aura(8.0f * pulse);
            aura.setOrigin(8.0f * pulse, 8.0f * pulse);
            aura.setPosition(e.position);
            aura.setFillColor(sf::Color(255, 215, 0, 50));
            target.draw(aura);

            sf::RectangleShape crystal(sf::Vector2f(8.0f * pulse, 8.0f * pulse));
            crystal.setOrigin(4.0f * pulse, 4.0f * pulse);
            crystal.setPosition(e.position);
            crystal.setFillColor(sf::Color(255, 230, 0));
            crystal.setOutlineThickness(1.0f);
            crystal.setOutlineColor(sf::Color::White);
            crystal.setRotation(time * 90.0f);
            target.draw(crystal);
        } else if (e.type == EntityType::Bullet) {
            // Bullet glow head
            sf::CircleShape head(3.0f);
            head.setOrigin(3.0f, 3.0f);
            head.setPosition(e.position);
            head.setFillColor(sf::Color(255, 255, 200));
            target.draw(head);

            // Bullet trail
            float rad = e.rotation * 3.14159f / 180.0f;
            sf::Vector2f dir(std::cos(rad), std::sin(rad));
            sf::Vertex trail[] = {
                sf::Vertex(e.position, sf::Color(255, 100, 0, 255)),
                sf::Vertex(e.position - dir * 18.0f, sf::Color(255, 0, 0, 0))
            };
            target.draw(trail, 2, sf::Lines);
        } else if (e.type == EntityType::Exit) {
            // Exit portal - Green glow
            sf::CircleShape glow(TILE_SIZE / 1.6f + 3.0f * std::sin(time * 4.0f));
            glow.setOrigin(glow.getRadius(), glow.getRadius());
            glow.setPosition(e.position);
            glow.setFillColor(sf::Color(0, 255, 100, 30));
            target.draw(glow);

            // Double rotating portal frames
            sf::RectangleShape outerFrame(sf::Vector2f(TILE_SIZE - 6.0f, TILE_SIZE - 6.0f));
            outerFrame.setOrigin(TILE_SIZE / 2.0f - 3.0f, TILE_SIZE / 2.0f - 3.0f);
            outerFrame.setPosition(e.position);
            outerFrame.setFillColor(sf::Color::Transparent);
            outerFrame.setOutlineThickness(2.0f);
            outerFrame.setOutlineColor(sf::Color(0, 255, 128));
            outerFrame.setRotation(time * 45.0f);
            target.draw(outerFrame);

            sf::RectangleShape innerFrame(sf::Vector2f(TILE_SIZE - 18.0f, TILE_SIZE - 18.0f));
            innerFrame.setOrigin(TILE_SIZE / 2.0f - 9.0f, TILE_SIZE / 2.0f - 9.0f);
            innerFrame.setPosition(e.position);
            innerFrame.setFillColor(sf::Color::Transparent);
            innerFrame.setOutlineThickness(1.5f);
            innerFrame.setOutlineColor(sf::Color(150, 255, 200));
            innerFrame.setRotation(-time * 90.0f);
            target.draw(innerFrame);

            // Portal core
            sf::CircleShape core(8.0f);
            core.setOrigin(8.0f, 8.0f);
            core.setPosition(e.position);
            core.setFillColor(sf::Color(255, 255, 255, 200));
            target.draw(core);
        }
    }

    // Draw Self Player Ship
    // Bottom glow
    sf::CircleShape selfGlow(18.0f);
    selfGlow.setOrigin(18.0f, 18.0f);
    selfGlow.setPosition(state.myPosition);
    selfGlow.setFillColor(sf::Color(0, 255, 128, 40));
    target.draw(selfGlow);

    sf::ConvexShape selfShip;
    selfShip.setPointCount(4);
    selfShip.setPoint(0, sf::Vector2f(14.0f, 0.0f));
    selfShip.setPoint(1, sf::Vector2f(-10.0f, -8.0f));
    selfShip.setPoint(2, sf::Vector2f(-5.0f, 0.0f));
    selfShip.setPoint(3, sf::Vector2f(-10.0f, 8.0f));
    selfShip.setOrigin(0.0f, 0.0f);
    selfShip.setPosition(state.myPosition);
    selfShip.setRotation(state.myRotation);
    selfShip.setFillColor(sf::Color(0, 255, 128));
    selfShip.setOutlineThickness(1.5f);
    selfShip.setOutlineColor(sf::Color::White);
    target.draw(selfShip);

    // Direction line
    float selfRad = state.myRotation * 3.14159f / 180.0f;
    sf::Vector2f selfDir(std::cos(selfRad), std::sin(selfRad));
    sf::Vertex dirLine[] = {
        sf::Vertex(state.myPosition + selfDir * 14.0f, sf::Color(0, 255, 128, 200)),
        sf::Vertex(state.myPosition + selfDir * 35.0f, sf::Color(0, 255, 128, 0))
    };
    target.draw(dirLine, 2, sf::Lines);



    // Reset view for HUD rendering
    target.setView(target.getDefaultView());

    // Draw HUD
    if (m_hasFont) {
        // Left Column: Match Status & Stats
        sf::Text statusText;
        statusText.setFont(m_font);
        statusText.setCharacterSize(16);
        statusText.setFillColor(sf::Color(180, 200, 240));
        statusText.setPosition(20.0f, 20.0f);

        std::stringstream ss;
        if (state.serverState == ServerState::Lobby) {
            ss << "SYS STATE: LOBBY\n";
            ss << "STATUS:    WAITING FOR TEACHER\n";
        } else if (state.serverState == ServerState::Playing) {
            ss << "SYS STATE: PLAYING\n";
            ss << "ELAPSED:   " << (int)state.currentRoundTime << "s\n";
        } else {
            ss << "SYS STATE: ROUND OVER\n";
            ss << "NEXT ROUND IN: " << (int)state.currentRoundTime << "s\n";
        }
        statusText.setString(ss.str());
        target.draw(statusText);

        // Glassmorphic Scoreboard Panel (Top Right)
        sf::RectangleShape panel(sf::Vector2f(240.0f, 220.0f));
        panel.setPosition(540.0f, 20.0f);
        panel.setFillColor(sf::Color(12, 15, 23, 210));
        panel.setOutlineThickness(1.0f);
        panel.setOutlineColor(sf::Color(0, 150, 255, 100));
        target.draw(panel);

        // Bounding tech lines for scoreboard
        sf::Vertex techLines[] = {
            sf::Vertex(sf::Vector2f(540.0f, 55.0f), sf::Color(0, 150, 255, 100)),
            sf::Vertex(sf::Vector2f(780.0f, 55.0f), sf::Color(0, 150, 255, 100))
        };
        target.draw(techLines, 2, sf::Lines);

        sf::Text panelHeader;
        panelHeader.setFont(m_font);
        panelHeader.setString("LEADERBOARD");
        panelHeader.setCharacterSize(14);
        panelHeader.setStyle(sf::Text::Bold);
        panelHeader.setFillColor(sf::Color(0, 180, 255));
        panelHeader.setPosition(555.0f, 28.0f);
        target.draw(panelHeader);

        float scoreY = 65.0f;
        for (const auto& score : state.scoreboard) {
            if (scoreY > 215.0f) break; // Avoid overflowing panel
            
            sf::Text scoreText;
            scoreText.setFont(m_font);
            scoreText.setCharacterSize(12);
            scoreText.setFillColor(sf::Color(200, 220, 245));
            scoreText.setPosition(555.0f, scoreY);

            std::stringstream scoreSs;
            scoreSs << score.teamName << ": " << static_cast<int>(score.score) << " pts";
            
            scoreText.setString(scoreSs.str());
            target.draw(scoreText);
            scoreY += 20.0f;
        }

        // Stats Footer: Health and Ammo HUD (Bottom Left)
        // Health Title
        sf::Text hpTitle;
        hpTitle.setFont(m_font);
        hpTitle.setString("INTEGRITY");
        hpTitle.setCharacterSize(12);
        hpTitle.setFillColor(sf::Color(0, 255, 128));
        hpTitle.setPosition(20.0f, 700.0f);
        target.draw(hpTitle);

        // Health Bar Background
        sf::RectangleShape hpBg(sf::Vector2f(200.0f, 12.0f));
        hpBg.setPosition(20.0f, 720.0f);
        hpBg.setFillColor(sf::Color(40, 45, 55));
        hpBg.setOutlineThickness(1.0f);
        hpBg.setOutlineColor(sf::Color(60, 65, 75));
        target.draw(hpBg);

        // Health Bar Fill
        float hpPercentage = std::max(0.0f, std::min(100.0f, state.myHealth)) / 100.0f;
        sf::RectangleShape hpFill(sf::Vector2f(200.0f * hpPercentage, 12.0f));
        hpFill.setPosition(20.0f, 720.0f);
        hpFill.setFillColor(sf::Color(0, 255, 128));
        target.draw(hpFill);

        // Ammo Title
        sf::Text ammoTitle;
        ammoTitle.setFont(m_font);
        ammoTitle.setString("MUNITION");
        ammoTitle.setCharacterSize(12);
        ammoTitle.setFillColor(sf::Color(255, 200, 0));
        ammoTitle.setPosition(20.0f, 742.0f);
        target.draw(ammoTitle);

        // Ammo Tick Indicators (draw up to 15 visual ammo cells)
        int drawAmmo = std::min(15, state.myAmmo);
        for (int i = 0; i < drawAmmo; ++i) {
            sf::RectangleShape tick(sf::Vector2f(6.0f, 12.0f));
            tick.setPosition(20.0f + i * 9.0f, 760.0f);
            tick.setFillColor(sf::Color(255, 200, 0));
            target.draw(tick);
        }
        
        // Show numerical ammo count if larger or as text
        if (state.myAmmo > 0) {
            sf::Text ammoNum;
            ammoNum.setFont(m_font);
            ammoNum.setString("x" + std::to_string(state.myAmmo));
            ammoNum.setCharacterSize(12);
            ammoNum.setFillColor(sf::Color(255, 200, 0));
            ammoNum.setPosition(25.0f + drawAmmo * 9.0f, 758.0f);
            target.draw(ammoNum);
        } else {
            sf::Text outText;
            outText.setFont(m_font);
            outText.setString("OUT OF AMMO");
            outText.setCharacterSize(12);
            outText.setFillColor(sf::Color(255, 50, 50));
            outText.setPosition(20.0f, 760.0f);
            target.draw(outText);
        }
    }
}
