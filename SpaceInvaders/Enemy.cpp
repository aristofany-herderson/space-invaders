#include "Enemy.h"
#include "Constants.h"
#include "SoundManager.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

static float rnd(float lo, float hi)
{
    return lo + (hi - lo) * (std::rand() / float(RAND_MAX));
}
static int scoreFor(EnemyType t)
{
    switch (t)
    {
    case EnemyType::Drone:
        return Cfg::SCORE_DRONE;
    case EnemyType::Invader:
        return Cfg::SCORE_INVADER;
    case EnemyType::Brute:
        return Cfg::SCORE_BRUTE;
    case EnemyType::Kamikaze:
        return Cfg::SCORE_KAMIKAZE;
    case EnemyType::Elite:
        return Cfg::SCORE_ELITE;
    default:
        return 0;
    }
}
static sf::Color colFor(EnemyType t)
{
    switch (t)
    {
    case EnemyType::Drone:
        return Cfg::COL_DRONE;
    case EnemyType::Invader:
        return Cfg::COL_INVADER;
    case EnemyType::Brute:
        return Cfg::COL_BRUTE;
    case EnemyType::Kamikaze:
        return Cfg::COL_KAMIKAZE;
    case EnemyType::Elite:
        return Cfg::COL_ELITE;
    default:
        return Cfg::COL_UFO;
    }
}
static int hpFor(EnemyType t)
{
    if (t == EnemyType::Brute)
        return 2;
    if (t == EnemyType::Elite)
        return 3;
    return 1;
}

static EnemyType typeForRow(int row)
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

void EnemyManager::stopUFOLoop()
{
    if (m_ufoAlive)
    {
        SFX.stopLoop();
        m_ufoAlive = false;
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
    m_ufoTimer = rnd(Cfg::UFO_INTERVAL_MIN, Cfg::UFO_INTERVAL_MAX);
    m_aliveCount = 0;

    for (int r = 0; r < Cfg::FORMATION_ROWS; ++r)
    {
        EnemyType t = typeForRow(r);
        for (int c = 0; c < Cfg::FORMATION_COLS; ++c)
        {
            Enemy e;
            e.type = t;

            const auto &tex = GFX.enemy[static_cast<int>(t)];
            auto sz = tex.getSize();

            e.halfW = sz.x * 0.5f;
            e.halfH = sz.y * 0.5f;

            e.hp = hpFor(t);
            e.maxHp = e.hp;
            float bx = Cfg::FORM_START_X + c * Cfg::FORM_SPACING_X;
            float by = Cfg::FORM_START_Y + r * Cfg::FORM_SPACING_Y;
            e.formationBase = {bx, by};
            e.pos = {bx, by};
            e.speedMod = rnd(0.85f, 1.15f);
            e.phase = rnd(0.f, 6.28f);
            e.shootInterval = rnd(Cfg::ENEMY_SHOOT_MIN, Cfg::ENEMY_SHOOT_MAX) / std::max(1.f, float(wave) * Cfg::ENEMY_SHOOT_WAVE_SCL + 1.f);
            e.shootTimer = rnd(0.f, e.shootInterval);
            m_enemies.push_back(e);
            ++m_aliveCount;
        }
    }
}

PowerupType EnemyManager::randomDrop()
{
    float r = rnd(0.f, 1.f);
    if (r > Cfg::POWERUP_DROP_CHANCE)
        return PowerupType::None;
    float t = rnd(0.f, 1.f);
    if (t < 0.33f)
        return PowerupType::Rapid;
    if (t < 0.66f)
        return PowerupType::Triple;
    return PowerupType::Shield;
}

void EnemyManager::applyTypeMovement(Enemy &e, float dt, sf::Vector2f playerPos)
{
    e.phase += dt * e.speedMod;
    sf::Vector2f extra{0.f, 0.f};

    switch (e.type)
    {
    case EnemyType::Drone:
        extra.x = std::sin(e.phase * 3.2f) * 14.f;
        break;
    case EnemyType::Invader:
        break;
    case EnemyType::Brute:
        extra.x = std::sin(e.phase * 0.8f) * 8.f;
        extra.y = std::cos(e.phase * 0.6f) * 6.f;
        break;
    case EnemyType::Kamikaze:
        if (e.isDiving)
        {
            sf::Vector2f dir = playerPos - e.pos;
            float len = std::hypot(dir.x, dir.y);
            if (len > 1.f)
                dir /= len;
            e.pos += dir * 160.f * e.speedMod * dt;
            if (e.pos.y > float(Cfg::H) - 60.f)
            {
                e.isDiving = false;
                e.pos = e.formationBase;
                e.pos.x += m_formX;
            }
            return;
        }
        else
        {
            e.diveTimer -= dt;
            float diveChance = (Cfg::KAMIKAZE_DIVE_BASE + m_wave * Cfg::KAMIKAZE_DIVE_WAVE) * dt * 60.f;
            if (e.formationBase.y + m_stepY > float(Cfg::H) * 0.45f &&
                e.diveTimer <= 0.f && rnd(0.f, 1.f) < diveChance)
            {
                e.isDiving = true;
                e.diveTimer = rnd(6.f, 12.f);
            }
        }
        break;
    case EnemyType::Elite:
        extra.x = std::sin(e.phase * 1.1f) * 22.f;
        extra.y = std::sin(e.phase * 0.7f) * 10.f;
        break;
    default:
        break;
    }

    e.pos.x = e.formationBase.x + m_formX + extra.x;
    e.pos.y = e.formationBase.y + m_stepY + extra.y;
}

void EnemyManager::tryEnemyShoot(Enemy &e, BulletManager &bullets, float dt)
{
    e.shootTimer -= dt;
    if (e.shootTimer > 0.f)
        return;
    if (e.type == EnemyType::Kamikaze && e.isDiving)
        return;

    e.shootTimer = e.shootInterval;
    sf::Color col = colFor(e.type);

    bullets.spawnEnemy(e.pos, col);
    SFX.play("enemy_shoot", 55.f, 0.85f + rnd(0.f, 0.30f));

    if (e.type == EnemyType::Elite)
    {
        sf::Vector2f lv{-80.f, Cfg::ENEMY_BULLET_SPD};
        sf::Vector2f rv{80.f, Cfg::ENEMY_BULLET_SPD};
        float mag = std::hypot(lv.x, lv.y);
        lv /= mag / Cfg::ENEMY_BULLET_SPD;
        rv /= mag / Cfg::ENEMY_BULLET_SPD;
        bullets.getAll().push_back({e.pos, lv, BulletOwner::Enemy, true, col});
        bullets.getAll().push_back({e.pos, rv, BulletOwner::Enemy, true, col});
    }
}

void EnemyManager::update(float dt, sf::Vector2f playerPos,
                          BulletManager &bullets, ParticleSystem &fx,
                          HitCallback onKill, KamikazeHitCallback onKamikazeHit)
{
    float aliveCount = float(m_aliveCount);
    float speedBonus = (1.f + (Cfg::FORMATION_COLS * Cfg::FORMATION_ROWS - aliveCount) / (Cfg::FORMATION_COLS * Cfg::FORMATION_ROWS) * 1.4f);

    if (!m_stepping)
    {
        m_formX += m_formDir * m_formSpeed * speedBonus * dt;

        float minX = 1e9f, maxX = -1e9f;
        for (auto &e : m_enemies)
        {
            if (!e.alive)
                continue;
            float ex = e.formationBase.x + m_formX;
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
                e.formationBase.y += delta;
            m_stepY = 0.f;
            m_stepping = false;
        }
    }

    for (auto &e : m_enemies)
    {
        if (!e.alive)
        {
            if (e.exploding)
            {
                e.explodeTimer -= dt;
                if (e.explodeTimer <= 0.f)
                {
                    e.explodeTimer = 0.1f;
                    e.explodeFrame++;
                    if (e.explodeFrame >= 4)
                        e.exploding = false;
                }
            }
            continue;
        }
        if (e.hitFlash > 0.f)
            e.hitFlash -= dt;
        applyTypeMovement(e, dt, playerPos);
        tryEnemyShoot(e, bullets, dt);
    }

    for (auto &b : bullets.getAll())
    {
        if (!b.alive || b.owner != BulletOwner::Player)
            continue;

        for (auto &e : m_enemies)
        {
            if (!e.alive)
                continue;

            float dx = b.pos.x - e.pos.x;
            if (dx < 0.f)
                dx = -dx;
            if (dx > e.halfW)
                continue;

            float dy = b.pos.y - e.pos.y;
            if (dy < 0.f)
                dy = -dy;
            if (dy > e.halfH)
                continue;

            b.alive = false;
            e.hp--;
            e.hitFlash = 0.12f;
            fx.spawnHit(b.pos, colFor(e.type));

            if (e.hp <= 0)
            {
                e.alive = false;
                e.exploding = true;
                e.explodeTimer = 0.1f;
                fx.spawnExplosion(e.pos, colFor(e.type));
                onKill(e.pos, colFor(e.type), scoreFor(e.type));
                --m_aliveCount;

                PowerupType pu = randomDrop();
                if (pu != PowerupType::None)
                    m_powerups.push_back({e.pos, pu, true});
            }
            break;
        }
    }

    bool kamikazeDamageDealtThisFrame = false;

    for (auto &e : m_enemies)
    {
        if (!e.alive || e.type != EnemyType::Kamikaze || !e.isDiving)
            continue;

        float dx = e.pos.x - playerPos.x;
        float dy = e.pos.y - playerPos.y;
        float distSq = dx * dx + dy * dy;
        constexpr float KAMIKAZE_HIT_RADIUS = 22.f;

        if (distSq > KAMIKAZE_HIT_RADIUS * KAMIKAZE_HIT_RADIUS)
            continue;

        e.alive = false;
        e.isDiving = false;
        e.exploding = true;
        e.explodeTimer = 0.1f;
        fx.spawnExplosion(e.pos, colFor(EnemyType::Kamikaze));
        onKill(e.pos, colFor(EnemyType::Kamikaze), 0);
        --m_aliveCount;

        if (!kamikazeDamageDealtThisFrame)
        {
            kamikazeDamageDealtThisFrame = true;
            onKamikazeHit(e.pos);
        }
    }

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
    m_ufoAlive = true;
    m_ufoDir = (std::rand() % 2) ? 1.f : -1.f;
    m_ufoPos.x = (m_ufoDir > 0) ? -60.f : float(Cfg::W) + 60.f;
    m_ufoPos.y = Cfg::HUD_HEIGHT + 30.f;
    SFX.startLoop("ufo_loop", 60.f);
}

void EnemyManager::updateUFO(float dt, BulletManager &bullets,
                             ParticleSystem &fx, HitCallback onKill)
{
    if (!m_ufoAlive)
    {
        m_ufoTimer -= dt;
        if (m_ufoTimer <= 0.f)
        {
            spawnUFO();
            m_ufoTimer = rnd(Cfg::UFO_INTERVAL_MIN, Cfg::UFO_INTERVAL_MAX);
        }
        return;
    }

    m_ufoPos.x += m_ufoDir * Cfg::UFO_SPEED * dt;

    if (m_ufoPos.x < -80.f || m_ufoPos.x > float(Cfg::W) + 80.f)
    {
        m_ufoAlive = false;
        SFX.stopLoop();
        return;
    }

    for (auto &b : bullets.getAll())
    {
        if (b.owner != BulletOwner::Player || !b.alive)
            continue;
        float dx = b.pos.x - m_ufoPos.x;
        float dy = b.pos.y - m_ufoPos.y;

        if ((dx * dx + dy * dy) < (26.f * 26.f))
        {
            b.alive = false;
            m_ufoAlive = false;
            SFX.stopLoop();
            fx.spawnExplosion(m_ufoPos, Cfg::COL_UFO, 50);
            onKill(m_ufoPos, Cfg::COL_UFO, Cfg::UFO_SCORE);
            return;
        }
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
        if (!e.exploding)
            continue;
        int frame = std::min(e.explodeFrame, 3);
        sf::Sprite spr(GFX.explosion[frame]);
        auto sz = GFX.explosion[frame].getSize();
        spr.setOrigin({sz.x / 2.f, sz.y / 2.f});
        spr.setPosition(e.pos);
        rt.draw(spr);
    }

    std::array<int, 5> countByType{};
    for (const auto &e : m_enemies)
        if (e.alive && !e.exploding)
            countByType[static_cast<int>(e.type)]++;

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
            if (!e.alive || e.exploding)
                continue;
            if (static_cast<int>(e.type) != t)
                continue;

            sf::Color col = sf::Color::White;

            sf::Vector2f tl{e.pos.x - hw, e.pos.y - hh};
            sf::Vector2f tr{e.pos.x + hw, e.pos.y - hh};
            sf::Vector2f br{e.pos.x + hw, e.pos.y + hh};
            sf::Vector2f bl{e.pos.x - hw, e.pos.y + hh};

            m_vaEnemyBatch[vi + 0] = {tl, col, {0.f, 0.f}};
            m_vaEnemyBatch[vi + 1] = {tr, col, {float(tsz.x), 0.f}};
            m_vaEnemyBatch[vi + 2] = {br, col, {float(tsz.x), float(tsz.y)}};
            m_vaEnemyBatch[vi + 3] = {tl, col, {0.f, 0.f}};
            m_vaEnemyBatch[vi + 4] = {br, col, {float(tsz.x), float(tsz.y)}};
            m_vaEnemyBatch[vi + 5] = {bl, col, {0.f, float(tsz.y)}};
            vi += 6;

            if (e.maxHp > 1 && e.hp < e.maxHp)
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
                if (!e.alive || e.exploding)
                    continue;
                if (static_cast<int>(e.type) != t)
                    continue;
                if (e.maxHp <= 1 || e.hp >= e.maxHp)
                    continue;

                float ratio = float(e.hp) / float(e.maxHp);
                float bw = tsz.x * ratio;
                float bx = e.pos.x - hw;
                float by = e.pos.y + hh + 2.f;
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
    if (!m_ufoAlive)
        return;
    sf::Sprite spr(GFX.enemy[5]);
    auto sz = GFX.enemy[5].getSize();
    spr.setOrigin({sz.x / 2.f, sz.y / 2.f});
    spr.setPosition(m_ufoPos);

    float t = m_ufoPos.x / 100.f;
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
        if (e.alive && e.pos.y > float(Cfg::H) - Cfg::HUD_HEIGHT - 80.f)
            return true;
    return false;
}

sf::Vector2f EnemyManager::getLowestEnemyPos() const
{
    sf::Vector2f lowest{0.f, 0.f};
    for (const auto &e : m_enemies)
        if (e.alive && e.pos.y > lowest.y)
            lowest = e.pos;
    return lowest;
}