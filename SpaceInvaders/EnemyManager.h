#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>
#include "Enemy.h"
#include "Powerup.h"
#include "Particle.h"

class EnemyManager
{
public:
    using HitCallback = std::function<void(sf::Vector2f, sf::Color, int)>;
    using KamikazeHitCallback = std::function<void(sf::Vector2f)>;

    void spawnFormation(int wave);

    void update(float dt, sf::Vector2f playerPos, BulletManager &bullets, ParticleSystem &fx, HitCallback onKill, KamikazeHitCallback onKamikazeHit = [](sf::Vector2f) {});
    void draw(sf::RenderTarget &rt) const;

    bool allDead() const;
    bool hasReachedBottom() const;
    sf::Vector2f getLowestEnemyPos() const;

    void updateUFO(float dt, BulletManager &bullets,
                   ParticleSystem &fx, HitCallback onKill);
    void drawUFO(sf::RenderTarget &rt) const;

    void stopUFOLoop();

    std::vector<PowerUp> &getPowerups() { return m_powerups; }
    const std::vector<std::unique_ptr<Enemy>> &getEnemies() const { return m_enemies; }
    bool hasUFO() const { return m_ufo != nullptr; }

private:
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::unique_ptr<Enemy> m_ufo;
    std::vector<PowerUp> m_powerups;

    float m_formX = 0.f;
    float m_formDir = 1.f;
    float m_formSpeed = Cfg::FORM_SPEED_BASE;
    bool m_stepping = false;
    float m_stepY = 0.f;
    int m_wave = 1;

    float m_ufoTimer = 20.f;

    int m_aliveCount = 0;

    void spawnUFO();
    void updateFormation(float dt);
    void resolvePlayerBullets(BulletManager &bullets, ParticleSystem &fx, const HitCallback &onKill);
    void resolveRams(sf::Vector2f playerPos, ParticleSystem &fx, const HitCallback &onKill, const KamikazeHitCallback &onKamikazeHit);
    FormationState formationState() const;
    PowerupType randomDrop();

    mutable sf::VertexArray m_vaEnemyBatch;
    mutable sf::VertexArray m_vaHPBar;
};
