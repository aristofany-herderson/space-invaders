#include "Player.h"
#include "Constants.h"
#include "SpriteGen.h"
#include "SoundManager.h"
#include <algorithm>
#include <cmath>

void Player::init() {
    m_pos        = {float(Cfg::W) / 2.f, float(Cfg::H) - 70.f};
    m_lives      = Cfg::PLAYER_LIVES;
    m_score      = 0;
    m_shootTimer = 0.f;
    m_invulTimer = 0.f;
    m_blinkTimer = 0.f;
    m_visible    = true;
    m_rapidTimer = m_tripleTimer = m_shieldTimer = 0.f;
    m_shieldActive = false;
}

void Player::update(float dt, BulletManager& bullets, ParticleSystem& fx) {
    float speed = Cfg::PLAYER_SPEED;
    auto sz = GFX.player.getSize();
    float halfW = sz.x / 2.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        m_pos.x = std::max(halfW, m_pos.x - speed * dt);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        m_pos.x = std::min(float(Cfg::W) - halfW, m_pos.x + speed * dt);

    m_shootTimer -= dt;
    float cd = (m_rapidTimer > 0.f) ? Cfg::PLAYER_SHOOT_CD * 0.35f : Cfg::PLAYER_SHOOT_CD;
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) && m_shootTimer <= 0.f) {
        m_shootTimer = cd;
        sf::Vector2f muzzle = {m_pos.x, m_pos.y - sz.y / 2.f};
        if (m_tripleTimer > 0.f) bullets.spawnTriple(muzzle);
        else                     bullets.spawnPlayer(muzzle);
        SFX.play("shoot", 70.f, 0.95f + (float(std::rand() % 10) / 100.f));
    }
    
    if (m_lifeNotifTimer > 0.f) {
        m_lifeNotifTimer -= dt;
        m_lifeNotifAge += dt;
    }

    if (m_rapidTimer  > 0.f) m_rapidTimer  -= dt;
    if (m_tripleTimer > 0.f) m_tripleTimer -= dt;
    if (m_shieldTimer > 0.f) { m_shieldTimer -= dt; if (m_shieldTimer <= 0.f) m_shieldActive = false; }

    if (m_invulTimer > 0.f) {
        m_invulTimer -= dt;
        m_blinkTimer += dt;
        m_visible = (m_blinkTimer >= Cfg::PLAYER_BLINK)
            ? (m_blinkTimer = 0.f, !m_visible) : m_visible;
    } else {
        m_visible = true;
    }

    m_thrustPhase += dt * 12.f;
}

DamageResult Player::takeDamage(ParticleSystem& fx)
{
    if (m_invulTimer > 0.f || m_shieldActive)
        return DamageResult::Ignored;

    fx.spawnExplosion(m_pos, Cfg::COL_PLAYER, 20);
    SFX.play("player_hit", 85.f);

    m_invulTimer = Cfg::PLAYER_INVUL;
    m_blinkTimer = 0.f;

    m_lives--;

    if (m_lives <= 0)
        return DamageResult::Dead;

    return DamageResult::Damaged;
}

void Player::applyPowerup(PowerupType pu) {
    SFX.play("powerup", 80.f); 
    switch (pu) {
        case PowerupType::Rapid:  m_rapidTimer  = Cfg::POWERUP_DURATION; break;
        case PowerupType::Triple: m_tripleTimer = Cfg::POWERUP_DURATION; break;
        case PowerupType::Shield: m_shieldActive = true; m_shieldTimer = Cfg::POWERUP_DURATION; break;
        default: break;
    }
}

sf::FloatRect Player::getBounds() const {
    auto sz = GFX.player.getSize();
    return { {m_pos.x - sz.x / 2.f, m_pos.y - sz.y / 2.f}, {float(sz.x), float(sz.y)} };
}

void Player::draw(sf::RenderTarget& rt) const {
    if (!m_visible) return;

    float flameH = 8.f + std::sin(m_thrustPhase) * 5.f;
    auto sz = GFX.player.getSize();

    auto drawFlame = [&](float offsetX) {
        sf::RectangleShape flame({4.f, flameH});
        flame.setOrigin({2.f, 0.f});
        flame.setPosition({m_pos.x + offsetX, m_pos.y + sz.y / 2.f - 4.f});
        float t = std::sin(m_thrustPhase * 1.5f) * 0.5f + 0.5f;
        flame.setFillColor({255, static_cast<std::uint8_t>(100 + t * 155), 20, 200});
        rt.draw(flame);
        sf::RectangleShape core({2.f, flameH * 0.6f});
        core.setOrigin({1.f, 0.f});
        core.setPosition({m_pos.x + offsetX, m_pos.y + sz.y / 2.f - 4.f});
        core.setFillColor({255, 255, 200, 230});
        rt.draw(core);
    };
    drawFlame(-4.f);
    drawFlame( 4.f);
    drawFlame(-18.f);
    drawFlame( 18.f);

    sf::Sprite spr(GFX.player);
    spr.setOrigin({sz.x/2.f, sz.y/2.f});
    spr.setPosition(m_pos);
    rt.draw(spr);

    if (m_shieldActive) {
        sf::CircleShape bubble(sz.x * 0.7f);
        bubble.setOrigin({bubble.getRadius(), bubble.getRadius()});
        bubble.setPosition(m_pos);
        float pulse = std::sin(m_thrustPhase * 0.8f) * 0.5f + 0.5f;
        bubble.setFillColor({80, 150, 255, static_cast<std::uint8_t>(30 + pulse * 20)});
        bubble.setOutlineColor({100, 180, 255, static_cast<std::uint8_t>(150 + pulse * 80)});
        bubble.setOutlineThickness(2.f);
        rt.draw(bubble);
    }
}

void Player::addLife() {
    m_lives++;
    m_lifeNotifTimer = Cfg::LIFE_NOTIF_DURATION;
    m_lifeNotifAge = 0.f;
}

float Player::lifeNotifAlpha() const {
    constexpr float fadeIn = 0.3f;
    constexpr float fadeOut = 0.5f;
    float t = m_lifeNotifTimer;
    float age = Cfg::LIFE_NOTIF_DURATION - t;

    float alpha = 1.f;
    if (age < fadeIn)  alpha = age / fadeIn;
    if (t < fadeOut) alpha = t / fadeOut;
    return std::clamp(alpha, 0.f, 1.f) * 255.f;
}

sf::Vector2f Player::lifeNotifPos() const {
    float age = Cfg::LIFE_NOTIF_DURATION - m_lifeNotifTimer;
    return { m_pos.x, m_pos.y - 30.f - age * Cfg::LIFE_NOTIF_RISE };
}