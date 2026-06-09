#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>

struct Star {
    sf::Vector2f pos;
    float        speed;
    float        brightness;
    float        size;
};

class Background {
public:
    Background();

    void update(float dt, float shakeX = 0.f, float shakeY = 0.f);
    void draw(sf::RenderTarget& rt) const;
    void setScrollSpeed(float s) { m_scrollMul = s; }

private:
    void rebuildVerts();

    std::vector<Star>           m_stars;
    sf::VertexBuffer            m_starBuffer{ sf::PrimitiveType::Triangles,
                                              sf::VertexBuffer::Usage::Stream };
    std::vector<sf::Vertex>     m_starVertsCpu;

    std::array<float, 3>        m_nebRadii{};
    float                       m_nebulaX = 0.f;
    float                       m_scrollMul = 1.f;

    bool                        m_vertsDirty = true;
};