#pragma once
#include <SFML/Graphics.hpp>
#include "Protocol.hpp"

class Visualizer {
public:
    Visualizer();
    void render(sf::RenderTarget& target, const VisibleState& state);
private:
    sf::Font m_font;
    bool m_hasFont;
};
