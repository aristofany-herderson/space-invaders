#pragma once
#include <SFML/Graphics.hpp>
#include "Spritegen.h"
#include "Bullet.h"
#include "Constants.h"
#include "Damageable.h"

struct FormationState
{
    float offsetX = 0.f;
    float stepY = 0.f;
    int wave = 1;
};

class Enemy : public Damageable
{
public:
    Enemy(EnemyType type, sf::Vector2f base, int hp, int wave);
    ~Enemy() override = default;

    void update(float dt, sf::Vector2f playerPos, const FormationState &form, BulletManager &bullets);
    void updateExplosion(float dt);
    void shiftBase(float dy) { m_base.y += dy; }

    DamageResult takeDamage(int amount, sf::Vector2f impact) override;
    bool isAlive() const override { return m_alive; }
    bool isHitBy(sf::Vector2f point, float radius) const override;
    sf::FloatRect getBounds() const override;

    virtual int scoreValue() const = 0;
    virtual sf::Color color() const = 0;
    virtual bool rams(sf::Vector2f playerPos) const;
    virtual bool isGone() const { return false; }

    EnemyType type() const { return m_type; }
    sf::Vector2f pos() const { return m_pos; }
    sf::Vector2f formationBase() const { return m_base; }
    int hp() const { return m_hp; }
    int maxHp() const { return m_maxHp; }
    bool isExploding() const { return m_exploding; }
    int explodeFrame() const { return m_explodeFrame; }

protected:
    virtual void move(float dt, sf::Vector2f playerPos, const FormationState &form) = 0;
    virtual void attack(BulletManager &bullets) = 0;
    virtual bool canAttack() const { return true; }

    void place(const FormationState &form, sf::Vector2f extra);
    void fireStraight(BulletManager &bullets);

    sf::Vector2f m_pos;
    sf::Vector2f m_base;
    float m_phase = 0.f;
    float m_speedMod = 1.f;

private:
    void shoot(float dt, BulletManager &bullets);
    void kill();

    EnemyType m_type;
    float m_halfW = 16.f;
    float m_halfH = 16.f;
    int m_hp = 1;
    int m_maxHp = 1;
    bool m_alive = true;
    float m_shootTimer = 0.f;
    float m_shootInterval = 2.f;
    float m_hitFlash = 0.f;
    bool m_exploding = false;
    float m_explodeTimer = 0.f;
    int m_explodeFrame = 0;
};

class Drone : public Enemy
{
public:
    Drone(sf::Vector2f base, int wave) : Enemy(EnemyType::Drone, base, 1, wave) {}

    int scoreValue() const override { return Cfg::SCORE_DRONE; }
    sf::Color color() const override { return Cfg::COL_DRONE; }

protected:
    void move(float dt, sf::Vector2f playerPos, const FormationState &form) override;
    void attack(BulletManager &bullets) override;
};

class Invader : public Enemy
{
public:
    Invader(sf::Vector2f base, int wave) : Enemy(EnemyType::Invader, base, 1, wave) {}

    int scoreValue() const override { return Cfg::SCORE_INVADER; }
    sf::Color color() const override { return Cfg::COL_INVADER; }

protected:
    void move(float dt, sf::Vector2f playerPos, const FormationState &form) override;
    void attack(BulletManager &bullets) override;
};

class Brute : public Enemy
{
public:
    Brute(sf::Vector2f base, int wave) : Enemy(EnemyType::Brute, base, 2, wave) {}

    int scoreValue() const override { return Cfg::SCORE_BRUTE; }
    sf::Color color() const override { return Cfg::COL_BRUTE; }

protected:
    void move(float dt, sf::Vector2f playerPos, const FormationState &form) override;
    void attack(BulletManager &bullets) override;
};

class Kamikaze : public Enemy
{
public:
    Kamikaze(sf::Vector2f base, int wave) : Enemy(EnemyType::Kamikaze, base, 1, wave) {}

    int scoreValue() const override { return Cfg::SCORE_KAMIKAZE; }
    sf::Color color() const override { return Cfg::COL_KAMIKAZE; }
    bool rams(sf::Vector2f playerPos) const override;
    bool isDiving() const { return m_diving; }

protected:
    void move(float dt, sf::Vector2f playerPos, const FormationState &form) override;
    void attack(BulletManager &bullets) override;
    bool canAttack() const override { return !m_diving; }

private:
    float m_diveTimer = 0.f;
    bool m_diving = false;
};

class Elite : public Enemy
{
public:
    Elite(sf::Vector2f base, int wave) : Enemy(EnemyType::Elite, base, 3, wave) {}

    int scoreValue() const override { return Cfg::SCORE_ELITE; }
    sf::Color color() const override { return Cfg::COL_ELITE; }

protected:
    void move(float dt, sf::Vector2f playerPos, const FormationState &form) override;
    void attack(BulletManager &bullets) override;
};

class UFO : public Enemy
{
public:
    UFO(sf::Vector2f start, int wave)
        : Enemy(EnemyType::UFO, start, 1, wave), m_dir(start.x < 0.f ? 1.f : -1.f) {}

    int scoreValue() const override { return Cfg::UFO_SCORE; }
    sf::Color color() const override { return Cfg::COL_UFO; }
    bool isHitBy(sf::Vector2f point, float radius) const override;
    bool isGone() const override;

protected:
    void move(float dt, sf::Vector2f playerPos, const FormationState &form) override;
    void attack(BulletManager &) override {}
    bool canAttack() const override { return false; }

private:
    float m_dir;
};
