#include "Background.h"
#include "Constants.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

static float rnd(float lo, float hi)
{
    return lo + (hi - lo) * (std::rand() / float(RAND_MAX));
}

Background::Background()
{
    for (int i = 0; i < 120; ++i)
        m_stars.push_back({{rnd(0, Cfg::W), rnd(0, Cfg::H)},
                           rnd(10.f, 20.f),
                           rnd(60.f, 130.f),
                           rnd(0.8f, 1.4f)});

    for (int i = 0; i < 70; ++i)
        m_stars.push_back({{rnd(0, Cfg::W), rnd(0, Cfg::H)},
                           rnd(22.f, 45.f),
                           rnd(140.f, 200.f),
                           rnd(1.2f, 2.0f)});

    for (int i = 0; i < 30; ++i)
        m_stars.push_back({{rnd(0, Cfg::W), rnd(0, Cfg::H)},
                           rnd(50.f, 90.f),
                           rnd(200.f, 255.f),
                           rnd(1.6f, 2.8f)});

    m_starVertsCpu.resize(m_stars.size() * 6);
    m_starBuffer.create(m_starVertsCpu.size());

    m_nebRadii = {rnd(85, 125), rnd(85, 125), rnd(85, 125)};

    m_bgRect.setSize({float(Cfg::W), float(Cfg::H)});
    m_bgRect.setFillColor({4, 4, 18, 255});

    static const std::array<sf::Color, 3> nebCol = {
        sf::Color{40, 10, 80, 22}, sf::Color{10, 30, 70, 18}, sf::Color{60, 10, 40, 16}};
    for (int i = 0; i < 3; ++i)
    {
        m_nebCircles[i].setRadius(m_nebRadii[i]);
        m_nebCircles[i].setOrigin({m_nebCircles[i].getRadius(),
                                   m_nebCircles[i].getRadius()});
        m_nebCircles[i].setFillColor(nebCol[i]);
    }

    rebuildVerts();
}

void Background::rebuildVerts()
{
    for (std::size_t i = 0; i < m_stars.size(); ++i)
    {
        const Star &s = m_stars[i];
        float r = s.size;
        std::uint8_t b = static_cast<std::uint8_t>(s.brightness);
        sf::Color col{b, b, static_cast<std::uint8_t>(std::min(255, int(b) + 30)), 255};
        std::size_t vi = i * 6;
        sf::Vector2f tl{s.pos.x - r, s.pos.y - r}, tr{s.pos.x + r, s.pos.y - r};
        sf::Vector2f br{s.pos.x + r, s.pos.y + r}, bl{s.pos.x - r, s.pos.y + r};
        m_starVertsCpu[vi + 0] = {tl, col};
        m_starVertsCpu[vi + 1] = {tr, col};
        m_starVertsCpu[vi + 2] = {br, col};
        m_starVertsCpu[vi + 3] = {tl, col};
        m_starVertsCpu[vi + 4] = {br, col};
        m_starVertsCpu[vi + 5] = {bl, col};
    }
    m_starBuffer.update(m_starVertsCpu.data(), m_starVertsCpu.size(), 0);
    m_vertsDirty = false;
}

void Background::update(float dt, float /*shakeX*/, float /*shakeY*/)
{
    m_nebulaX += dt * 4.f;

    bool anyMoved = false;
    for (auto &s : m_stars)
    {
        float oldY = s.pos.y;
        s.pos.y += s.speed * m_scrollMul * dt;
        if (s.pos.y > Cfg::H + 4.f)
        {
            s.pos.y = -4.f;
            s.pos.x = rnd(0, Cfg::W);
        }

        if (std::abs(s.pos.y - oldY) > 0.5f)
            anyMoved = true;
    }

    if (anyMoved)
        m_vertsDirty = true;

    if (m_vertsDirty)
        rebuildVerts();
}

void Background::draw(sf::RenderTarget &rt) const
{
    rt.draw(m_bgRect);

    std::array<sf::Vector2f, 3> nebPos = {
        sf::Vector2f{150.f + std::sin(m_nebulaX * 0.07f) * 20.f, 200.f},
        sf::Vector2f{600.f + std::cos(m_nebulaX * 0.05f) * 15.f, 350.f},
        sf::Vector2f{350.f + std::sin(m_nebulaX * 0.09f) * 18.f, 120.f}};
    for (int i = 0; i < 3; ++i)
    {
        m_nebCircles[i].setPosition(nebPos[i]);
        rt.draw(m_nebCircles[i]);
    }

    rt.draw(m_starBuffer, 0, m_starVertsCpu.size());
}