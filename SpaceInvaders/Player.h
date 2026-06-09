#pragma once
#include <SFML/Graphics.hpp>
#include "Bullet.h"
#include "Particle.h"
#include "Enemy.h"

enum class PowerupType;

enum class DamageResult {
    Ignored,
    Damaged,
    Dead
};

class Player {
public:
    void init();
    void update(float dt, BulletManager& bullets, ParticleSystem& fx);
    void draw(sf::RenderTarget& rt) const;

    DamageResult takeDamage(ParticleSystem& fx);
    void applyPowerup(PowerupType pu);

    sf::FloatRect getBounds() const;
    sf::Vector2f  getPos()    const { return m_pos; }
    int           getLives()  const { return m_lives; }
    int           getScore()  const { return m_score; }
    void          addScore(int s) { m_score += s; }
    bool          isInvul()   const { return m_invulTimer > 0.f; }
    bool          isAlive()   const { return m_lives > 0; }

    float getRapidTimer()  const { return m_rapidTimer; }
    float getTripleTimer() const { return m_tripleTimer; }
    float getShieldTimer() const { return m_shieldTimer; }
    bool  hasShield()      const { return m_shieldActive; }

    void addLife();                      

    bool  hasLifeNotif()   const { return m_lifeNotifTimer > 0.f; }
    float lifeNotifAlpha() const;       
    sf::Vector2f lifeNotifPos() const;

private:
    sf::Vector2f m_pos;
    int          m_lives        = Cfg::PLAYER_LIVES;
    int          m_score        = 0;
    float        m_shootTimer   = 0.f;
    float        m_invulTimer   = 0.f;
    float        m_blinkTimer   = 0.f;
    bool         m_visible      = true;

  
    float        m_rapidTimer   = 0.f;
    float        m_tripleTimer  = 0.f;
    bool         m_shieldActive = false;
    float        m_shieldTimer  = 0.f;

    float        m_thrustPhase  = 0.f;

    float        m_lifeNotifTimer   = 0.f;
    float        m_lifeNotifAge     = 0.f;
};