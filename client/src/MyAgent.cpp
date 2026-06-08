#include "Agent.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

class MyAgent : public Agent {
public:
    MyAgent(const std::string& teamName) : m_teamName(teamName) {}

    std::string getTeamName() const override {
        return m_teamName;
    }

    void update(const VisibleState& state) override {
        m_state = state;
    }

    Action getAction() override {
        // Tady budete implementovat svou logiku agenta!
        // Nyní je agent ovládán pouze klávesnicí pro testovací účely.

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            return Action::MoveForward;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            return Action::MoveBackward;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            return Action::RotateLeft;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            return Action::RotateRight;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
            return Action::PickupAmmo;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            return Action::Shoot;
        }

        return Action::None;
    }

private:
    VisibleState m_state;
    std::string m_teamName;
};

// Tovární funkce, kterou klient používá k získání vašeho agenta
Agent* createStudentAgent(const std::string& teamName) {
    return new MyAgent(teamName);
}
