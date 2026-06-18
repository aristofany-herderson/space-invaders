#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Particle
{
    sf::Vector2f pos, vel;
    sf::Color color;
    float life, maxLife, size;
};

struct FloatingText
{
    sf::Vector2f pos, vel;
    std::string text;
    sf::Color color;
    float life, maxLife;
    unsigned fontSize;
};

class ParticleSystem
{
public:
    void spawnExplosion(sf::Vector2f pos, sf::Color col, int count = 38);
    void spawnHit(sf::Vector2f pos, sf::Color col);
    void addFloatingScore(sf::Vector2f pos, int score, sf::Color col);
    void update(float dt);
    void draw(sf::RenderTarget &rt, const sf::Font &font) const;
    void clear();

private:
    mutable sf::VertexArray m_verts;
    std::vector<Particle> m_particles;
    std::vector<FloatingText> m_texts;
};
