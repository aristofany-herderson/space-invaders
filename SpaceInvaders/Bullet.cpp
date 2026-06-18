#include "Bullet.h"
#include "Constants.h"
#include "SpriteGen.h"
#include <algorithm>

void BulletManager::spawn(sf::Vector2f pos, sf::Vector2f vel, BulletOwner owner)
{
    m_bullets.push_back({ pos, vel, owner, true });
}

void BulletManager::spawnPlayer(sf::Vector2f pos)
{
    spawn(pos, { 0.f, -Cfg::PLAYER_BULLET_SPD }, BulletOwner::Player);
}

void BulletManager::spawnEnemy(sf::Vector2f pos, sf::Color color)
{
    Bullet b;
    b.pos = pos;
    b.vel = { 0.f, Cfg::ENEMY_BULLET_SPD };
    b.owner = BulletOwner::Enemy;
    b.alive = true;
    b.color = color;
    m_bullets.push_back(b);
}

void BulletManager::spawnTriple(sf::Vector2f pos)
{
    spawn(pos, { -80.f, -Cfg::PLAYER_BULLET_SPD * 0.95f }, BulletOwner::Player);
    spawn(pos, { 0.f,  -Cfg::PLAYER_BULLET_SPD }, BulletOwner::Player);
    spawn(pos, { 80.f, -Cfg::PLAYER_BULLET_SPD * 0.95f }, BulletOwner::Player);
}

void BulletManager::update(float dt)
{
    for (auto& b : m_bullets) {
        b.pos += b.vel * dt;
        if (b.pos.y < -20.f || b.pos.y > float(Cfg::H) + 20.f ||
            b.pos.x < -20.f || b.pos.x > float(Cfg::W) + 20.f)
            b.alive = false;
    }
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const Bullet& b) { return !b.alive; }),
        m_bullets.end());
}

static inline void writeQuad(sf::VertexArray& va, std::size_t vi,
    sf::Vector2f pos, float hw, float hh,
    sf::Color col,
    sf::Vector2f uvTL = {}, sf::Vector2f uvBR = {})
{
    sf::Vector2f tl{ pos.x - hw, pos.y - hh };
    sf::Vector2f tr{ pos.x + hw, pos.y - hh };
    sf::Vector2f br{ pos.x + hw, pos.y + hh };
    sf::Vector2f bl{ pos.x - hw, pos.y + hh };

    sf::Vector2f uvTR{ uvBR.x, uvTL.y };
    sf::Vector2f uvBL{ uvTL.x, uvBR.y };

    va[vi + 0] = { tl, col, uvTL };
    va[vi + 1] = { tr, col, uvTR };
    va[vi + 2] = { br, col, uvBR };
    va[vi + 3] = { tl, col, uvTL };
    va[vi + 4] = { br, col, uvBR };
    va[vi + 5] = { bl, col, uvBL };
}

void BulletManager::draw(sf::RenderTarget& rt) const
{
    if (m_bullets.empty()) return;

    m_vaPlayer.setPrimitiveType(sf::PrimitiveType::Triangles);
    m_vaEnemy.setPrimitiveType(sf::PrimitiveType::Triangles);
    m_vaTrail.setPrimitiveType(sf::PrimitiveType::Triangles);

    std::size_t nP = 0, nE = 0;
    for (const auto& b : m_bullets) {
        if (b.owner == BulletOwner::Player) ++nP;
        else                                ++nE;
    }
    const std::size_t nT = nP + nE;

    m_vaPlayer.resize(nP * 6);
    m_vaEnemy.resize(nE * 6);
    m_vaTrail.resize(nT * 6);

    const auto& texP = GFX.bulletPlayer;
    const auto& texE = GFX.bulletEnemyBase;
    sf::Vector2u szP = texP.getSize();
    sf::Vector2u szE = texE.getSize();

    const float hwP = szP.x / 2.f, hhP = szP.y / 2.f;
    const float hwE = szE.x / 2.f, hhE = szE.y / 2.f;

    sf::Vector2f uvP_TL{ 0.f, 0.f };
    sf::Vector2f uvP_BR{ float(szP.x), float(szP.y) };
    sf::Vector2f uvE_TL{ 0.f, 0.f };
    sf::Vector2f uvE_BR{ float(szE.x), float(szE.y) };

    std::size_t viP = 0, viE = 0, viT = 0;

    for (const auto& b : m_bullets) {
        if (b.owner == BulletOwner::Player) {
            writeQuad(m_vaPlayer, viP, b.pos, hwP, hhP,
                sf::Color::White, uvP_TL, uvP_BR);
            viP += 6;

            sf::Color tc{ 80, 200, 255, 70 };
            sf::Vector2f trailPos{ b.pos.x, b.pos.y + hhP + 5.f };
            writeQuad(m_vaTrail, viT, trailPos, 1.5f, 6.f, tc);
            viT += 6;
        }
        else {
            writeQuad(m_vaEnemy, viE, b.pos, hwE, hhE,
                b.color, uvE_TL, uvE_BR);
            viE += 6;

            sf::Color tc{ b.color.r, b.color.g, b.color.b, 70 };
            sf::Vector2f trailPos{ b.pos.x, b.pos.y - hhE - 5.f };
            writeQuad(m_vaTrail, viT, trailPos, 1.5f, 6.f, tc);
            viT += 6;
        }
    }

    if (viT > 0)
        rt.draw(&m_vaTrail[0], viT, sf::PrimitiveType::Triangles);

    if (viP > 0) {
        sf::RenderStates stateP;
        stateP.texture = &texP;
        rt.draw(&m_vaPlayer[0], viP, sf::PrimitiveType::Triangles, stateP);
    }

    if (viE > 0) {
        sf::RenderStates stateE;
        stateE.texture = &texE;
        rt.draw(&m_vaEnemy[0], viE, sf::PrimitiveType::Triangles, stateE);
    }
}