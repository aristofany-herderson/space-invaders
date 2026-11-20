#pragma once
#include <SFML/Graphics.hpp>

enum class DamageResult
{
    Ignored,
    Damaged,
    Dead
};

class Damageable
{
public:
    virtual ~Damageable() = default;

    virtual DamageResult takeDamage(int amount, sf::Vector2f impact) = 0;
    virtual bool isAlive() const = 0;
    virtual bool isHitBy(sf::Vector2f point, float radius) const = 0;
    virtual sf::FloatRect getBounds() const = 0;
};
