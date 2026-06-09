#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include "SpriteGen.h"
#include "Bullet.h"
#include "Particle.h"
#include "Constants.h"

enum class PowerupType { None, Rapid, Triple, Shield };

class PowerUp {
public:
    sf::Vector2f pos;
    PowerupType  type;
    bool         alive = true;
};

struct Enemy {
    sf::Vector2f  pos;
    sf::Vector2f  formationBase;

    float         halfW = 16.f;
    float         halfH = 16.f;

    EnemyType     type          = EnemyType::Drone;
    int           hp            = 1;
    int           maxHp         = 1;
    bool          alive         = true;
    float         shootTimer    = 0.f;
    float         shootInterval = 2.f;
    float         phase         = 0.f;
    float         diveTimer     = 0.f;
    bool          isDiving      = false;
    sf::Vector2f  diveTarget;
    float         speedMod      = 1.f;
    float         hitFlash      = 0.f;
    bool          exploding     = false;
    float         explodeTimer  = 0.f;
    int           explodeFrame  = 0;
};

class EnemyManager {
public:
    using HitCallback = std::function<void(sf::Vector2f, sf::Color, int)>;
    using KamikazeHitCallback = std::function<void(sf::Vector2f)>;

    void spawnFormation(int wave);

    void update(float dt, sf::Vector2f playerPos,
        BulletManager& bullets, ParticleSystem& fx,
        HitCallback onKill,
        KamikazeHitCallback onKamikazeHit = [](sf::Vector2f) {});
    void draw(sf::RenderTarget& rt) const;

    bool         allDead() const;
    bool         hasReachedBottom() const;
    sf::Vector2f getLowestEnemyPos() const;

    // UFO
    void  updateUFO(float dt, BulletManager& bullets,
        ParticleSystem& fx, HitCallback onKill);
    void  drawUFO(sf::RenderTarget& rt) const;

    std::vector<PowerUp>& getPowerups() { return m_powerups; }

private:
    std::vector<Enemy>   m_enemies;
    std::vector<PowerUp> m_powerups;

    float m_formX       = 0.f;
    float m_formDir     = 1.f;
    float m_formSpeed   = Cfg::FORM_SPEED_BASE;
    bool  m_stepping    = false;
    float m_stepY       = 0.f;
    int   m_wave        = 1;

    bool         m_ufoAlive = false;
    sf::Vector2f m_ufoPos;
    float        m_ufoTimer = 20.f;
    float        m_ufoDir   = 1.f;

    void spawnUFO();
    void tryEnemyShoot(Enemy& e, BulletManager& bullets, float dt);
    void applyTypeMovement(Enemy& e, float dt, sf::Vector2f playerPos);
    PowerupType randomDrop();

    mutable sf::VertexArray m_vaEnemyBatch;
    mutable sf::VertexArray m_vaHPBar;
};