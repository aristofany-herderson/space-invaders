#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "SpriteGen.h"

struct ShieldBlock
{
    sf::Vector2f pos;
    int hp;
    bool alive;
};

class Shield
{
public:
    void init(sf::Vector2f topLeft);
    void draw(sf::RenderTarget &rt) const;

    bool checkBulletHit(sf::Vector2f bulletPos, float bulletRadius = 4.f);

    sf::FloatRect getBounds() const { return m_bounds; }
    const std::vector<ShieldBlock> &getBlocks() const { return m_blocks; }

private:
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
    void reset();

private:
    std::vector<Shield> m_shields;
    mutable sf::VertexArray m_shieldVerts;
    mutable bool m_shieldDirty = true;
};
