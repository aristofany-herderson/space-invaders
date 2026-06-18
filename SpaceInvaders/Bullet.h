#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

enum class BulletOwner { Player, Enemy };

class Bullet {
public:
    sf::Vector2f pos;
    sf::Vector2f vel;
    BulletOwner  owner;
    bool         alive = true;
    sf::Color    color{ 255, 80, 80, 255 };
};

class BulletManager {
public:
    void spawnPlayer(sf::Vector2f pos);

    void spawnEnemy(sf::Vector2f pos, sf::Color color = { 255, 80, 80, 255 });
    void spawnTriple(sf::Vector2f pos);

    void update(float dt);
    void draw(sf::RenderTarget& rt) const;

    void killAll() { m_bullets.clear(); }
    std::vector<Bullet>& getAll() { return m_bullets; }

private:
    std::vector<Bullet> m_bullets;

    void spawn(sf::Vector2f pos, sf::Vector2f vel, BulletOwner owner);

    mutable sf::VertexArray m_vaPlayer;
    mutable sf::VertexArray m_vaEnemy;
    mutable sf::VertexArray m_vaTrail;
};