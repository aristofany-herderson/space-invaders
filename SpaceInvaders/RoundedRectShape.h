#pragma once
#include <SFML/Graphics.hpp>
#include <cstddef>
#include <algorithm>

class RoundedRectShape : public sf::Shape
{
public:
    explicit RoundedRectShape(sf::Vector2f size = {100.f, 40.f},
                              float radius = 8.f,
                              std::size_t cornerPoints = 10u);

    void setSize(sf::Vector2f size);
    void setCornerRadius(float radius);
    void setCornerPointCount(std::size_t points);

    sf::Vector2f getSize() const { return m_size; }
    float getCornerRadius() const { return m_radius; }
    std::size_t getCornerPointCount() const { return m_points; }

    std::size_t getPointCount() const override;
    sf::Vector2f getPoint(std::size_t index) const override;

private:
    sf::Vector2f m_size;
    float m_radius;
    std::size_t m_points;

    static float clampRadius(float r, sf::Vector2f sz);
    void rebuild();
};