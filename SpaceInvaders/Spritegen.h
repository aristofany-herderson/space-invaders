#pragma once
#include <SFML/Graphics.hpp>
#include <array>

enum class EnemyType { Drone, Invader, Brute, Kamikaze, Elite, UFO };

struct TextureBank {
    sf::Texture                 player;
    sf::Texture                 playerThrust;
    sf::Texture                 bulletPlayer;
    sf::Texture                 bulletEnemy;
    sf::Texture                 bulletEnemyBase;
    std::array<sf::Texture, 6>  enemy;     
    std::array<sf::Texture, 4>  shield;    
    std::array<sf::Texture, 4>  explosion; 
    sf::Texture                 powerupRapid;
    sf::Texture                 powerupTriple;
    sf::Texture                 powerupShield;

    void generate();

private:
    static sf::Texture makePlayer();
    static sf::Texture makeBullet(sf::Color col, float w, float h);
    static sf::Texture makeEnemy(EnemyType t);
    static sf::Texture makeShieldTile(int damage);
    static sf::Texture makeExplosionFrame(int frame);
    static sf::Texture makePowerup(sf::Color col, const std::string& label);

    static bool tryLoad(sf::Texture& tex, const std::string& path);
};

extern TextureBank GFX;