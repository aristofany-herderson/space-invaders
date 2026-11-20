#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Spritegen.h"
#include "Damageable.h"

struct ShieldBlock
{
    sf::Vector2f pos;
    int hp;
    bool alive;
};

class Shield : public Damageable
{
public:
    void init(sf::Vector2f topLeft);
    void draw(sf::RenderTarget &rt) const;

    bool checkBulletHit(sf::Vector2f bulletPos, float bulletRadius = 4.f);

    DamageResult takeDamage(int amount, sf::Vector2f impact) override;
    bool isAlive() const override;
    bool isHitBy(sf::Vector2f point, float radius) const override;
    sf::FloatRect getBounds() const override { return m_bounds; }
    const std::vector<ShieldBlock> &getBlocks() const { return m_blocks; }

private:
    ShieldBlock *findBlock(sf::Vector2f point, float radius);
    const ShieldBlock *findBlock(sf::Vector2f point, float radius) const;
    DamageResult damageBlock(ShieldBlock &block, int amount);

    std::vector<ShieldBlock> m_blocks;
    sf::FloatRect m_bounds;
    float m_blockSize;
};

class ShieldManager
{
public:
    void init();
    void markDirty() { m_shieldDirty = true; }
    void draw(sf::RenderTarget &rt) const;
    bool checkBulletHit(sf::Vector2f pos, float r = 4.f);
    std::vector<Shield> &getShields() { return m_shields; }
    void reset();

private:
    std::vector<Shield> m_shields;
    mutable sf::VertexArray m_shieldVerts;
    mutable bool m_shieldDirty = true;
};
