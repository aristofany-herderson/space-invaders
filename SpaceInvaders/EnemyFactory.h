#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Enemy.h"

class EnemyFactory
{
public:
    EnemyFactory() = delete;

    static std::unique_ptr<Enemy> create(EnemyType type, sf::Vector2f pos, int wave);
    static std::unique_ptr<Enemy> createUfo(int wave);
    static std::vector<std::unique_ptr<Enemy>> createFormation(int wave);
    static EnemyType typeForRow(int row);
};
