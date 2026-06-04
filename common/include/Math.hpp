#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace Math {
    constexpr float PI = 3.14159265358979323846f;

    inline float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    inline sf::Vector2f getDirection(float rotationDegrees) {
        float rad = rotationDegrees * PI / 180.0f;
        return sf::Vector2f(std::cos(rad), std::sin(rad));
    }
}
