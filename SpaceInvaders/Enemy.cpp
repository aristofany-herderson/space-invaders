#include "Enemy.h"
#include "SoundManager.h"
#include "Random.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float KAMIKAZE_HIT_RADIUS = 22.f;
    constexpr float UFO_HIT_RADIUS = 26.f;
    constexpr float UFO_MARGIN = 80.f;
}

Enemy::Enemy(EnemyType type, sf::Vector2f base, int hp, int wave)
    : m_pos(base), m_base(base), m_type(type), m_hp(hp), m_maxHp(hp)
{
    auto sz = GFX.enemy[static_cast<std::size_t>(type)].getSize();
    if (sz.x > 0 && sz.y > 0)
    {
        m_halfW = sz.x * 0.5f;
        m_halfH = sz.y * 0.5f;
    }

    m_speedMod = Rng::range(0.85f, 1.15f);
    m_phase = Rng::range(0.f, 6.28f);
    m_shootInterval = Rng::range(Cfg::ENEMY_SHOOT_MIN, Cfg::ENEMY_SHOOT_MAX) /
                      std::max(1.f, float(wave) * Cfg::ENEMY_SHOOT_WAVE_SCL + 1.f);
    m_shootTimer = Rng::range(0.f, m_shootInterval);
}

void Enemy::update(float dt, sf::Vector2f playerPos, const FormationState &form, BulletManager &bullets)
{
    if (m_hitFlash > 0.f)
        m_hitFlash -= dt;

    m_phase += dt * m_speedMod;
    move(dt, playerPos, form);
    shoot(dt, bullets);
}

void Enemy::updateExplosion(float dt)
{
    if (!m_exploding)
        return;
    m_explodeTimer -= dt;
    if (m_explodeTimer > 0.f)
        return;
    m_explodeTimer = 0.1f;
    if (++m_explodeFrame >= 4)
        m_exploding = false;
}

DamageResult Enemy::takeDamage(int amount, sf::Vector2f)
{
    if (!m_alive || amount <= 0)
        return DamageResult::Ignored;

    m_hp -= amount;
    m_hitFlash = 0.12f;
    if (m_hp > 0)
        return DamageResult::Damaged;

    kill();
    return DamageResult::Dead;
}

bool Enemy::isHitBy(sf::Vector2f point, float radius) const
{
    return std::abs(point.x - m_pos.x) <= m_halfW + radius &&
           std::abs(point.y - m_pos.y) <= m_halfH + radius;
}

sf::FloatRect Enemy::getBounds() const
{
    return {{m_pos.x - m_halfW, m_pos.y - m_halfH}, {m_halfW * 2.f, m_halfH * 2.f}};
}

bool Enemy::rams(sf::Vector2f) const
{
    return false;
}

void Enemy::place(const FormationState &form, sf::Vector2f extra)
{
    m_pos.x = m_base.x + form.offsetX + extra.x;
    m_pos.y = m_base.y + form.stepY + extra.y;
}

void Enemy::fireStraight(BulletManager &bullets)
{
    bullets.spawnEnemy(m_pos, color());
}

void Enemy::shoot(float dt, BulletManager &bullets)
{
    m_shootTimer -= dt;
    if (m_shootTimer > 0.f || !canAttack())
        return;

    m_shootTimer = m_shootInterval;
    attack(bullets);
    SFX.play(SoundId::EnemyShoot, 55.f, 0.85f + Rng::range(0.f, 0.30f));
}

void Enemy::kill()
{
    m_alive = false;
    m_exploding = true;
    m_explodeTimer = 0.1f;
    m_explodeFrame = 0;
}

void Drone::move(float, sf::Vector2f, const FormationState &form)
{
    place(form, {std::sin(m_phase * 3.2f) * 14.f, 0.f});
}

void Drone::attack(BulletManager &bullets)
{
    fireStraight(bullets);
}

void Invader::move(float, sf::Vector2f, const FormationState &form)
{
    place(form, {0.f, 0.f});
}

void Invader::attack(BulletManager &bullets)
{
    fireStraight(bullets);
}

void Brute::move(float, sf::Vector2f, const FormationState &form)
{
    place(form, {std::sin(m_phase * 0.8f) * 8.f, std::cos(m_phase * 0.6f) * 6.f});
}

void Brute::attack(BulletManager &bullets)
{
    fireStraight(bullets);
}

void Kamikaze::move(float dt, sf::Vector2f playerPos, const FormationState &form)
{
    if (m_diving)
    {
        sf::Vector2f dir = playerPos - m_pos;
        float len = std::hypot(dir.x, dir.y);
        if (len > 1.f)
            dir /= len;
        m_pos += dir * 160.f * m_speedMod * dt;
        if (m_pos.y > float(Cfg::H) - 60.f)
        {
            m_diving = false;
            m_pos = m_base;
            m_pos.x += form.offsetX;
        }
        return;
    }

    m_diveTimer -= dt;
    float diveChance = (Cfg::KAMIKAZE_DIVE_BASE + form.wave * Cfg::KAMIKAZE_DIVE_WAVE) * dt * 60.f;
    if (m_base.y + form.stepY > float(Cfg::H) * 0.45f &&
        m_diveTimer <= 0.f && Rng::range(0.f, 1.f) < diveChance)
    {
        m_diving = true;
        m_diveTimer = Rng::range(6.f, 12.f);
    }
    place(form, {0.f, 0.f});
}

void Kamikaze::attack(BulletManager &bullets)
{
    fireStraight(bullets);
}

bool Kamikaze::rams(sf::Vector2f playerPos) const
{
    if (!isAlive() || !m_diving)
        return false;
    sf::Vector2f d = m_pos - playerPos;
    return d.x * d.x + d.y * d.y <= KAMIKAZE_HIT_RADIUS * KAMIKAZE_HIT_RADIUS;
}

void Elite::move(float, sf::Vector2f, const FormationState &form)
{
    place(form, {std::sin(m_phase * 1.1f) * 22.f, std::sin(m_phase * 0.7f) * 10.f});
}

void Elite::attack(BulletManager &bullets)
{
    fireStraight(bullets);

    sf::Vector2f left{-80.f, Cfg::ENEMY_BULLET_SPD};
    sf::Vector2f right{80.f, Cfg::ENEMY_BULLET_SPD};
    float mag = std::hypot(left.x, left.y);
    left /= mag / Cfg::ENEMY_BULLET_SPD;
    right /= mag / Cfg::ENEMY_BULLET_SPD;
    bullets.getAll().push_back({m_pos, left, BulletOwner::Enemy, true, color()});
    bullets.getAll().push_back({m_pos, right, BulletOwner::Enemy, true, color()});
}

void UFO::move(float dt, sf::Vector2f, const FormationState &)
{
    m_pos.x += m_dir * Cfg::UFO_SPEED * dt;
}

bool UFO::isHitBy(sf::Vector2f point, float radius) const
{
    sf::Vector2f d = point - m_pos;
    float r = UFO_HIT_RADIUS + radius;
    return d.x * d.x + d.y * d.y < r * r;
}

bool UFO::isGone() const
{
    return m_pos.x < -UFO_MARGIN || m_pos.x > float(Cfg::W) + UFO_MARGIN;
}
