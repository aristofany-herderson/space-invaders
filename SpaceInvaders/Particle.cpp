#include "Particle.h"
#include "Constants.h"
#include <cstdlib>
#include <cmath>
#include <sstream>

static float rnd(float lo, float hi)
{
    return lo + (hi - lo) * (std::rand() / float(RAND_MAX));
}
static float rndAngle() { return rnd(0, 2.f * 3.14159f); }

void ParticleSystem::spawnExplosion(sf::Vector2f pos, sf::Color col, int count)
{
    for (int i = 0; i < count; ++i)
    {
        float ang = rndAngle();
        float spd = rnd(40.f, 220.f);
        float life = rnd(0.4f, Cfg::PART_LIFETIME);
        float sz = rnd(1.5f, 5.f);
        sf::Color c = col;
        c.a = 255;
        if (i < count / 4)
        {
            c.r = 255;
            c.g = 255;
            c.b = 220;
            sz += 1.5f;
        }
        m_particles.push_back({pos,
                               {std::cos(ang) * spd, std::sin(ang) * spd},
                               c,
                               life,
                               life,
                               sz});
    }
}

void ParticleSystem::spawnHit(sf::Vector2f pos, sf::Color col)
{
    for (int i = 0; i < 10; ++i)
    {
        float ang = rndAngle();
        float spd = rnd(20.f, 80.f);
        float life = rnd(0.15f, 0.4f);
        m_particles.push_back({pos,
                               {std::cos(ang) * spd, std::sin(ang) * spd},
                               col,
                               life,
                               life,
                               rnd(1.f, 3.f)});
    }
}

void ParticleSystem::addFloatingScore(sf::Vector2f pos, int score, sf::Color col)
{
    std::string s = "+" + std::to_string(score);
    m_texts.push_back({pos, {0.f, -42.f}, s, col, 1.2f, 1.2f, 14u});
}

void ParticleSystem::update(float dt)
{
    for (auto &p : m_particles)
    {
        p.life -= dt;
        p.pos += p.vel * dt;
        p.vel *= (1.f - dt * 2.2f);
        float ratio = p.life / p.maxLife;
        p.color.a = static_cast<std::uint8_t>(255 * ratio);
        p.size = p.size * (0.92f + ratio * 0.08f);
    }
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
                       [](const Particle &p)
                       { return p.life <= 0; }),
        m_particles.end());

    for (auto &t : m_texts)
    {
        t.life -= dt;
        t.pos += t.vel * dt;
        t.vel *= (1.f - dt * 3.f);
        float ratio = t.life / t.maxLife;
        t.color.a = static_cast<std::uint8_t>(255 * ratio);
    }
    m_texts.erase(
        std::remove_if(m_texts.begin(), m_texts.end(),
                       [](const FloatingText &t)
                       { return t.life <= 0; }),
        m_texts.end());
}

void ParticleSystem::draw(sf::RenderTarget &rt, const sf::Font &font) const
{
    if (!m_particles.empty())
    {
        const std::size_t needed = m_particles.size() * 6;
        if (m_verts.getVertexCount() != needed)
        {
            m_verts.setPrimitiveType(sf::PrimitiveType::Triangles);
            m_verts.resize(needed);
        }

        for (std::size_t i = 0; i < m_particles.size(); ++i)
        {
            const auto &p = m_particles[i];
            float r = p.size * 0.5f;
            std::size_t vi = i * 6;
            sf::Vector2f tl{p.pos.x - r, p.pos.y - r}, tr{p.pos.x + r, p.pos.y - r};
            sf::Vector2f br{p.pos.x + r, p.pos.y + r}, bl{p.pos.x - r, p.pos.y + r};
            m_verts[vi + 0] = {tl, p.color};
            m_verts[vi + 1] = {tr, p.color};
            m_verts[vi + 2] = {br, p.color};
            m_verts[vi + 3] = {tl, p.color};
            m_verts[vi + 4] = {br, p.color};
            m_verts[vi + 5] = {bl, p.color};
        }
        rt.draw(m_verts);
    }

    for (const auto &t : m_texts)
    {
        sf::Text txt(font, t.text, t.fontSize);
        txt.setFillColor(t.color);
        txt.setOutlineColor({0, 0, 0, t.color.a});
        txt.setOutlineThickness(1.f);
        auto b = txt.getLocalBounds();
        txt.setOrigin({b.size.x / 2.f, b.size.y / 2.f});
        txt.setPosition(t.pos);
        rt.draw(txt);
    }
}

void ParticleSystem::clear()
{
    m_particles.clear();
    m_texts.clear();
    m_verts.resize(0);
}
