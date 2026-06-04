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
        
        // This is where students will write their logic.
        // For example, they can check state.entities for enemies, ammo, and walls.
    }

    Action getAction() override {
        // By default, we provide manual keyboard controls so students can playtest.
        // They should replace this with automatic AI logic!
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) return Action::MoveForward;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) return Action::MoveBackward;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) return Action::RotateLeft;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) return Action::RotateRight;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) return Action::PickupAmmo;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) return Action::Shoot;
        
        return Action::None;
    }

private:
    VisibleState m_state;
    std::string m_teamName;
};

// Factory function used by the client to get the student's agent
Agent* createStudentAgent(const std::string& teamName) {
    return new MyAgent(teamName);
}
