#include "Shield.h"
#include "Constants.h"
#include <cmath>
#include <utility>

void Shield::init(sf::Vector2f topLeft)
{
    m_blockSize = Cfg::SHIELD_BLOCK;
    m_blocks.clear();
    float totalW = Cfg::SHIELD_COLS * m_blockSize;
    float totalH = Cfg::SHIELD_ROWS * m_blockSize;
    m_bounds = {
        topLeft,
        {totalW, totalH}};

    const char *shape[] = {
        "0001111000",
        "0011111100",
        "0111111110",
        "1111111111",
        "1111111111",
        "1111001111",
        "1110000111"};

    for (int r = 0; r < Cfg::SHIELD_ROWS; ++r)
    {
        for (int c = 0; c < Cfg::SHIELD_COLS; ++c)
        {

            if (shape[r][c] != '1')
                continue;

            sf::Vector2f pos = {
                topLeft.x + c * m_blockSize,
                topLeft.y + r * m_blockSize};

            m_blocks.push_back({pos, 3, true});
        }
    }
}

void Shield::draw(sf::RenderTarget &rt) const
{
    for (const auto &b : m_blocks)
    {
        if (!b.alive)
            continue;
        int dmg = 3 - b.hp;
        if (dmg >= 4)
            dmg = 3;
        sf::Sprite spr(GFX.shield[dmg]);
        spr.setPosition(b.pos);
        rt.draw(spr);
    }
}

const ShieldBlock *Shield::findBlock(sf::Vector2f point, float radius) const
{
    for (const auto &b : m_blocks)
    {
        if (!b.alive)
            continue;
        sf::FloatRect br{b.pos, {m_blockSize, m_blockSize}};
        if (br.contains(point) ||
            std::hypot(point.x - (b.pos.x + m_blockSize / 2.f),
                       point.y - (b.pos.y + m_blockSize / 2.f)) < radius + m_blockSize / 2.f)
            return &b;
    }
    return nullptr;
}

ShieldBlock *Shield::findBlock(sf::Vector2f point, float radius)
{
    return const_cast<ShieldBlock *>(std::as_const(*this).findBlock(point, radius));
}

DamageResult Shield::damageBlock(ShieldBlock &block, int amount)
{
    block.hp -= amount;
    if (block.hp <= 0)
        block.alive = false;
    return isAlive() ? DamageResult::Damaged : DamageResult::Dead;
}

bool Shield::checkBulletHit(sf::Vector2f bulletPos, float bulletRadius)
{
    ShieldBlock *block = findBlock(bulletPos, bulletRadius);
    if (!block)
        return false;
    damageBlock(*block, 1);
    return true;
}

DamageResult Shield::takeDamage(int amount, sf::Vector2f impact)
{
    ShieldBlock *block = findBlock(impact, Cfg::BULLET_RADIUS);
    if (!block || amount <= 0)
        return DamageResult::Ignored;
    return damageBlock(*block, amount);
}

bool Shield::isAlive() const
{
    for (const auto &b : m_blocks)
        if (b.alive)
            return true;
    return false;
}

bool Shield::isHitBy(sf::Vector2f point, float radius) const
{
    return findBlock(point, radius) != nullptr;
}

void ShieldManager::init()
{
    m_shields.clear();
    float spacing = float(Cfg::W) / (Cfg::SHIELD_COUNT + 1);
    float shieldW = Cfg::SHIELD_COLS * Cfg::SHIELD_BLOCK;
    float y = float(Cfg::H) - 160.f;
    for (int i = 0; i < Cfg::SHIELD_COUNT; ++i)
    {
        Shield s;
        float x = spacing * (i + 1) - shieldW / 2.f;
        s.init({x, y});
        m_shields.push_back(std::move(s));
    }
}

void ShieldManager::draw(sf::RenderTarget &rt) const
{
    if (m_shieldDirty)
    {
        std::size_t total = 0;
        for (const auto &s : m_shields)
            for (const auto &b : s.getBlocks())
                if (b.alive)
                    ++total;

        m_shieldVerts.setPrimitiveType(sf::PrimitiveType::Triangles);
        m_shieldVerts.resize(total * 6);

        static const sf::Color dmgCol[4] = {
            {0, 255, 100, 240},
            {160, 255, 50, 200},
            {255, 200, 20, 160},
            {255, 80, 20, 100}};

        std::size_t vi = 0;
        float bs = Cfg::SHIELD_BLOCK;
        for (const auto &s : m_shields)
        {
            for (const auto &b : s.getBlocks())
            {
                if (!b.alive)
                    continue;
                int dmg = std::min(3 - b.hp, 3);
                sf::Color col = dmgCol[std::max(0, dmg)];

                sf::Vector2f tl = b.pos;
                sf::Vector2f tr{b.pos.x + bs, b.pos.y};
                sf::Vector2f br{b.pos.x + bs, b.pos.y + bs};
                sf::Vector2f bl{b.pos.x, b.pos.y + bs};

                m_shieldVerts[vi + 0] = {tl, col};
                m_shieldVerts[vi + 1] = {tr, col};
                m_shieldVerts[vi + 2] = {br, col};
                m_shieldVerts[vi + 3] = {tl, col};
                m_shieldVerts[vi + 4] = {br, col};
                m_shieldVerts[vi + 5] = {bl, col};
                vi += 6;
            }
        }
        m_shieldDirty = false;
    }
    rt.draw(m_shieldVerts);
}

bool ShieldManager::checkBulletHit(sf::Vector2f pos, float r)
{
    for (auto &s : m_shields)
    {
        if (s.checkBulletHit(pos, r))
        {
            m_shieldDirty = true;
            return true;
        }
    }
    return false;
}

void ShieldManager::reset()
{
    init();
    m_shieldDirty = true;
}
