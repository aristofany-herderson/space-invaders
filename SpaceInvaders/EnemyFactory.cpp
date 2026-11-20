#include "EnemyFactory.h"
#include "Constants.h"
#include "Random.h"
#include <stdexcept>

std::unique_ptr<Enemy> EnemyFactory::create(EnemyType type, sf::Vector2f pos, int wave)
{
    switch (type)
    {
    case EnemyType::Drone:
        return std::make_unique<Drone>(pos, wave);
    case EnemyType::Invader:
        return std::make_unique<Invader>(pos, wave);
    case EnemyType::Brute:
        return std::make_unique<Brute>(pos, wave);
    case EnemyType::Kamikaze:
        return std::make_unique<Kamikaze>(pos, wave);
    case EnemyType::Elite:
        return std::make_unique<Elite>(pos, wave);
    case EnemyType::UFO:
        return std::make_unique<UFO>(pos, wave);
    }
    throw std::invalid_argument("tipo de inimigo desconhecido");
}

std::unique_ptr<Enemy> EnemyFactory::createUfo(int wave)
{
    float x = Rng::coin() ? -60.f : float(Cfg::W) + 60.f;
    return create(EnemyType::UFO, {x, Cfg::HUD_HEIGHT + 30.f}, wave);
}

std::vector<std::unique_ptr<Enemy>> EnemyFactory::createFormation(int wave)
{
    std::vector<std::unique_ptr<Enemy>> formation;
    formation.reserve(static_cast<std::size_t>(Cfg::FORMATION_ROWS * Cfg::FORMATION_COLS));

    for (int r = 0; r < Cfg::FORMATION_ROWS; ++r)
    {
        EnemyType type = typeForRow(r);
        for (int c = 0; c < Cfg::FORMATION_COLS; ++c)
        {
            sf::Vector2f pos{Cfg::FORM_START_X + c * Cfg::FORM_SPACING_X,
                             Cfg::FORM_START_Y + r * Cfg::FORM_SPACING_Y};
            formation.push_back(create(type, pos, wave));
        }
    }
    return formation;
}

EnemyType EnemyFactory::typeForRow(int row)
{
    switch (row)
    {
    case 0:
        return EnemyType::Elite;
    case 1:
        return EnemyType::Kamikaze;
    case 2:
        return EnemyType::Brute;
    case 3:
        return EnemyType::Drone;
    default:
        return EnemyType::Invader;
    }
}
