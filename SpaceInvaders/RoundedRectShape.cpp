#include "RoundedRectShape.h"
#include <cmath>
#include <algorithm>

static constexpr float kPi = 3.14159265358979323846f;

float RoundedRectShape::clampRadius(float r, sf::Vector2f sz)
{
    return std::min(r, std::min(sz.x, sz.y) * 0.5f);
}

RoundedRectShape::RoundedRectShape(sf::Vector2f size,
                                   float radius,
                                   std::size_t cornerPoints)
    : m_size(size), m_radius(clampRadius(radius, size)), m_points(std::max(cornerPoints, std::size_t(2)))
{
    rebuild();
}

void RoundedRectShape::setSize(sf::Vector2f size)
{
    m_size = size;
    m_radius = clampRadius(m_radius, size);
    rebuild();
}

void RoundedRectShape::setCornerRadius(float radius)
{
    m_radius = clampRadius(radius, m_size);
    rebuild();
}

void RoundedRectShape::setCornerPointCount(std::size_t points)
{
    m_points = std::max(points, std::size_t(2));
    rebuild();
}

std::size_t RoundedRectShape::getPointCount() const
{
    return m_points * 4u;
}

sf::Vector2f RoundedRectShape::getPoint(std::size_t index) const
{
    struct Corner
    {
        float cx, cy, startDeg;
    };

    const Corner corners[4] = {
        {m_radius, m_radius, 180.f},
        {m_size.x - m_radius, m_radius, 270.f},
        {m_size.x - m_radius, m_size.y - m_radius, 0.f},
        {m_radius, m_size.y - m_radius, 90.f}};

    const std::size_t corner = index / m_points;
    const std::size_t i = index % m_points;

    const float startRad = corners[corner].startDeg * kPi / 180.f;
    const float angle = startRad + (kPi * 0.5f) * float(i) / float(m_points);

    return {
        corners[corner].cx + std::cos(angle) * m_radius,
        corners[corner].cy + std::sin(angle) * m_radius};
}

void RoundedRectShape::rebuild()
{
    sf::Shape::update();
}