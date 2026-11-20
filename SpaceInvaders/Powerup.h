#pragma once
#include <SFML/Graphics.hpp>

enum class PowerupType
{
    None,
    Rapid,
    Triple,
    Shield
};

class PowerUp
{
public:
    sf::Vector2f pos;
    PowerupType type;
    bool alive = true;
};
