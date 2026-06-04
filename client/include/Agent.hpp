#pragma once
#include "Protocol.hpp"
#include <string>

// This is the base class that students will inherit from.
class Agent {
public:
    virtual ~Agent() = default;

    // Provide the team name to be displayed on the scoreboard
    virtual std::string getTeamName() const = 0;

    // Called every game tick with the latest information from the server
    virtual void update(const VisibleState& state) = 0;

    // Called after update() to get the agent's desired action
    virtual Action getAction() = 0;
};
