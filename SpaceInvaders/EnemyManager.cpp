#include "EnemyManager.h"
#include "EnemyFactory.h"
#include "Constants.h"
#include "SoundManager.h"
#include "Random.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

void EnemyManager::stopUFOLoop()
{
    if (m_ufo)
    {
        SFX.stopLoop();
        m_ufo.reset();
    }
}

void EnemyManager::spawnFormation(int wave)
{
    stopUFOLoop();

    m_enemies.clear();
    m_powerups.clear();
    m_formX = 0.f;
    m_formDir = 1.f;
    m_stepping = false;
    m_stepY = 0.f;
    m_wave = wave;
    m_formSpeed = Cfg::FORM_SPEED_BASE +
                  (wave - 1) * Cfg::FORM_SPEED_SCALE * 5.5f;
    m_ufoTimer = Rng::range(Cfg::UFO_INTERVAL_MIN, Cfg::UFO_INTERVAL_MAX);

    m_enemies = EnemyFactory::createFormation(wave);
    m_aliveCount = static_cast<int>(m_enemies.size());
}

PowerupType EnemyManager::randomDrop()
{
    float r = Rng::range(0.f, 1.f);
    if (r > Cfg::POWERUP_DROP_CHANCE)
        return PowerupType::None;
    float t = Rng::range(0.f, 1.f);
    if (t < 0.33f)
        return PowerupType::Rapid;
    if (t < 0.66f)
        return PowerupType::Triple;
    return PowerupType::Shield;
}

FormationState EnemyManager::formationState() const
{
    return {m_formX, m_stepY, m_wave};
}

void EnemyManager::updateFormation(float dt)
{
    float aliveCount = float(m_aliveCount);
    float speedBonus = (1.f + (Cfg::FORMATION_COLS * Cfg::FORMATION_ROWS - aliveCount) / (Cfg::FORMATION_COLS * Cfg::FORMATION_ROWS) * 1.4f);

    if (!m_stepping)
    {
        m_formX += m_formDir * m_formSpeed * speedBonus * dt;

        float minX = 1e9f, maxX = -1e9f;
        for (const auto &e : m_enemies)
        {
            if (!e->isAlive())
                continue;
            float ex = e->formationBase().x + m_formX;
            minX = std::min(minX, ex - 18.f);
            maxX = std::max(maxX, ex + 18.f);
        }
        if (maxX >= float(Cfg::W) - 20.f && m_formDir > 0)
        {
            m_formDir = -1.f;
            m_stepping = true;
        }
        else if (minX <= 20.f && m_formDir < 0)
        {
            m_formDir = 1.f;
            m_stepping = true;
        }
    }
    else
    {
        m_stepY += 10.f * dt;
        if (m_stepY >= Cfg::FORM_STEP_V)
        {
            float delta = m_stepY;
            for (auto &e : m_enemies)
                e->shiftBase(delta);
            m_stepY = 0.f;
            m_stepping = false;
        }
    }
}

void EnemyManager::resolvePlayerBullets(BulletManager &bullets, ParticleSystem &fx, const HitCallback &onKill)
{
    for (auto &b : bullets.getAll())
    {
        if (!b.alive || b.owner != BulletOwner::Player)
            continue;

        for (auto &e : m_enemies)
        {
            if (!e->isAlive() || !e->isHitBy(b.pos, 0.f))
                continue;

            b.alive = false;
            DamageResult result = e->takeDamage(1, b.pos);
            fx.spawnHit(b.pos, e->color());

            if (result == DamageResult::Dead)
            {
                fx.spawnExplosion(e->pos(), e->color());
                onKill(e->pos(), e->color(), e->scoreValue());
                --m_aliveCount;

                PowerupType pu = randomDrop();
                if (pu != PowerupType::None)
                    m_powerups.push_back({e->pos(), pu, true});
            }
            break;
        }
    }
}

void EnemyManager::resolveRams(sf::Vector2f playerPos, ParticleSystem &fx, const HitCallback &onKill, const KamikazeHitCallback &onKamikazeHit)
{
    bool damageDealtThisFrame = false;

    for (auto &e : m_enemies)
    {
        if (!e->isAlive() || !e->rams(playerPos))
            continue;

        e->takeDamage(e->hp(), e->pos());
        fx.spawnExplosion(e->pos(), e->color());
        onKill(e->pos(), e->color(), 0);
        --m_aliveCount;

        if (!damageDealtThisFrame)
        {
            damageDealtThisFrame = true;
            onKamikazeHit(e->pos());
        }
    }
}

void EnemyManager::update(float dt, sf::Vector2f playerPos,
                          BulletManager &bullets, ParticleSystem &fx,
                          HitCallback onKill, KamikazeHitCallback onKamikazeHit)
{
    updateFormation(dt);

    FormationState form = formationState();
    for (auto &e : m_enemies)
    {
        if (!e->isAlive())
        {
            e->updateExplosion(dt);
            continue;
        }
        e->update(dt, playerPos, form, bullets);
    }

    resolvePlayerBullets(bullets, fx, onKill);
    resolveRams(playerPos, fx, onKill, onKamikazeHit);

    for (auto &p : m_powerups)
    {
        if (!p.alive)
            continue;
        p.pos.y += Cfg::POWERUP_SPEED * dt;
        if (p.pos.y > float(Cfg::H) + 30.f)
            p.alive = false;
    }

    updateUFO(dt, bullets, fx, onKill);
}

void EnemyManager::spawnUFO()
{
    m_ufo = EnemyFactory::createUfo(m_wave);
    SFX.startLoop(SoundId::UfoLoop, 60.f);
}

void EnemyManager::updateUFO(float dt, BulletManager &bullets,
                             ParticleSystem &fx, HitCallback onKill)
{
    if (!m_ufo)
    {
        m_ufoTimer -= dt;
        if (m_ufoTimer <= 0.f)
        {
            spawnUFO();
            m_ufoTimer = Rng::range(Cfg::UFO_INTERVAL_MIN, Cfg::UFO_INTERVAL_MAX);
        }
        return;
    }

    m_ufo->update(dt, {0.f, 0.f}, formationState(), bullets);

    if (m_ufo->isGone())
    {
        stopUFOLoop();
        return;
    }

    for (auto &b : bullets.getAll())
    {
        if (b.owner != BulletOwner::Player || !b.alive)
            continue;
        if (!m_ufo->isHitBy(b.pos, 0.f))
            continue;

        b.alive = false;
        sf::Vector2f pos = m_ufo->pos();
        int score = m_ufo->scoreValue();
        stopUFOLoop();
        fx.spawnExplosion(pos, Cfg::COL_UFO, 50);
        onKill(pos, Cfg::COL_UFO, score);
        return;
    }
}

void EnemyManager::draw(sf::RenderTarget &rt) const
{
    for (const auto &p : m_powerups)
    {
        if (!p.alive)
            continue;
        const sf::Texture &tex =
            (p.type == PowerupType::Rapid) ? GFX.powerupRapid : (p.type == PowerupType::Triple) ? GFX.powerupTriple
                                                                                                : GFX.powerupShield;
        sf::Sprite spr(tex);
        auto sz = tex.getSize();
        spr.setOrigin({sz.x / 2.f, sz.y / 2.f});
        spr.setPosition(p.pos);
        rt.draw(spr);
    }

    for (const auto &e : m_enemies)
    {
        if (!e->isExploding())
            continue;
        int frame = std::min(e->explodeFrame(), 3);
        sf::Sprite spr(GFX.explosion[frame]);
        auto sz = GFX.explosion[frame].getSize();
        spr.setOrigin({sz.x / 2.f, sz.y / 2.f});
        spr.setPosition(e->pos());
        rt.draw(spr);
    }

    std::array<int, 5> countByType{};
    for (const auto &e : m_enemies)
        if (e->isAlive())
            countByType[static_cast<int>(e->type())]++;

    m_vaEnemyBatch.setPrimitiveType(sf::PrimitiveType::Triangles);
    m_vaHPBar.setPrimitiveType(sf::PrimitiveType::Triangles);

    for (int t = 0; t < 5; ++t)
    {
        if (countByType[t] == 0)
            continue;

        const sf::Texture &tex = GFX.enemy[t];
        auto tsz = tex.getSize();
        float hw = tsz.x / 2.f, hh = tsz.y / 2.f;

        m_vaEnemyBatch.resize(static_cast<std::size_t>(countByType[t]) * 6);
        std::size_t vi = 0;

        int hpCount = 0;

        for (const auto &e : m_enemies)
        {
            if (!e->isAlive())
                continue;
            if (static_cast<int>(e->type()) != t)
                continue;

            sf::Color col = sf::Color::White;
            sf::Vector2f pos = e->pos();

            sf::Vector2f tl{pos.x - hw, pos.y - hh};
            sf::Vector2f tr{pos.x + hw, pos.y - hh};
            sf::Vector2f br{pos.x + hw, pos.y + hh};
            sf::Vector2f bl{pos.x - hw, pos.y + hh};

            m_vaEnemyBatch[vi + 0] = {tl, col, {0.f, 0.f}};
            m_vaEnemyBatch[vi + 1] = {tr, col, {float(tsz.x), 0.f}};
            m_vaEnemyBatch[vi + 2] = {br, col, {float(tsz.x), float(tsz.y)}};
            m_vaEnemyBatch[vi + 3] = {tl, col, {0.f, 0.f}};
            m_vaEnemyBatch[vi + 4] = {br, col, {float(tsz.x), float(tsz.y)}};
            m_vaEnemyBatch[vi + 5] = {bl, col, {0.f, float(tsz.y)}};
            vi += 6;

            if (e->maxHp() > 1 && e->hp() < e->maxHp())
                ++hpCount;
        }

        sf::RenderStates rs;
        rs.texture = &tex;
        rt.draw(m_vaEnemyBatch, rs);

        if (hpCount > 0)
        {
            m_vaHPBar.resize(static_cast<std::size_t>(hpCount) * 6);
            std::size_t hi = 0;
            for (const auto &e : m_enemies)
            {
                if (!e->isAlive())
                    continue;
                if (static_cast<int>(e->type()) != t)
                    continue;
                if (e->maxHp() <= 1 || e->hp() >= e->maxHp())
                    continue;

                float ratio = float(e->hp()) / float(e->maxHp());
                float bw = tsz.x * ratio;
                float bx = e->pos().x - hw;
                float by = e->pos().y + hh + 2.f;
                sf::Color hcol{20, 255, 80, 200};

                m_vaHPBar[hi + 0] = {{bx, by}, hcol};
                m_vaHPBar[hi + 1] = {{bx + bw, by}, hcol};
                m_vaHPBar[hi + 2] = {{bx + bw, by + 3.f}, hcol};
                m_vaHPBar[hi + 3] = {{bx, by}, hcol};
                m_vaHPBar[hi + 4] = {{bx + bw, by + 3.f}, hcol};
                m_vaHPBar[hi + 5] = {{bx, by + 3.f}, hcol};
                hi += 6;
            }
            rt.draw(m_vaHPBar);
        }
    }
}

void EnemyManager::drawUFO(sf::RenderTarget &rt) const
{
    if (!m_ufo)
        return;
    sf::Sprite spr(GFX.enemy[5]);
    auto sz = GFX.enemy[5].getSize();
    spr.setOrigin({sz.x / 2.f, sz.y / 2.f});
    spr.setPosition(m_ufo->pos());

    float t = m_ufo->pos().x / 100.f;
    std::uint8_t bri = static_cast<std::uint8_t>(200 + std::sin(t * 4.f) * 55.f);
    spr.setColor({bri, bri, bri, 255});
    rt.draw(spr);
}

bool EnemyManager::allDead() const
{
    return m_aliveCount <= 0;
}

bool EnemyManager::hasReachedBottom() const
{
    for (const auto &e : m_enemies)
        if (e->isAlive() && e->pos().y > float(Cfg::H) - Cfg::HUD_HEIGHT - 80.f)
            return true;
    return false;
}

sf::Vector2f EnemyManager::getLowestEnemyPos() const
{
    sf::Vector2f lowest{0.f, 0.f};
    for (const auto &e : m_enemies)
        if (e->isAlive() && e->pos().y > lowest.y)
            lowest = e->pos();
    return lowest;
}
