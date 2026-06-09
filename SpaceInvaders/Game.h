#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Background.h"
#include "SoundManager.h"
#include "Shield.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"
#include "Particle.h"
#include "SpriteGen.h"
#include <optional>

enum class GameState { Menu, Playing, Paused, GameOver, Win };

class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow m_window;
    sf::Font         m_font;
    sf::Clock        m_clock;
    GameState        m_state    = GameState::Menu;
    int              m_wave     = 1;
    int              m_hiScore  = 0;

    Background     m_bg;
    ShieldManager  m_shields;
    BulletManager  m_bullets;
    EnemyManager   m_enemies;
    Player         m_player;
    ParticleSystem m_fx;

    float     m_shakeTimer  = 0.f;
    float     m_shakeMag    = 0.f;
    float     m_flashTimer  = 0.f;
    sf::Color m_flashCol    = sf::Color::Transparent;
    float     m_bgScrollSpd = 1.f;

    float m_comboTimer      = 0.f;
    int   m_combo           = 0;
    float m_waveTextTimer   = 0.f;

    float         m_menuPulse = 0.f;
    sf::FloatRect m_playButtonRect{};

    void processEvents();
    void update(float dt);
    void render();

    void startGame();
    void nextWave();
    void triggerShake(float mag = Cfg::SHAKE_MAG, float dur = Cfg::SHAKE_DUR);
    void triggerFlash(sf::Color col, float dur = Cfg::FLASH_DUR);

    void drawHUD();
    void drawMenu();
    void drawPause();
    void drawGameOver();
    void drawWin();
    void drawLives();
    void drawPowerupHUD();

    void loadHighScore();
    void saveHighScore();

    void checkPlayerBulletVsShields();
    void checkEnemyBulletVsPlayer();
    void checkEnemyBulletVsShields();
    void checkPowerupVsPlayer();
    void checkEnemyVsPlayer();
};