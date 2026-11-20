#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include "Background.h"
#include "SoundManager.h"
#include "Shield.h"
#include "Bullet.h"
#include "EnemyManager.h"
#include "Player.h"
#include "Particle.h"
#include "Spritegen.h"
#include "RoundedRectShape.h"
#include "RankingClient.h"
#include <optional>
#include <future>

enum class GameState
{
    Menu,
    Countdown,
    Playing,
    Paused,
    GameOver,
    Win,
    NameEntry,
    Ranking
};

class Game
{
public:
    Game();
    void run();

private:
    sf::RenderWindow m_window;
    sf::Font m_font;
    sf::Clock m_clock;
    GameState m_state = GameState::Menu;
    int m_wave = 1;
    int m_hiScore = 0;
    float m_volume = 80.f;
    bool m_volumeDragging = false;
    sf::FloatRect m_volTrackRect;

    void loadSettings();
    void saveSettings();

    void drawVolumeSlider(float cx, float cy, float width = 130.f);

    Background m_bg;
    ShieldManager m_shields;
    BulletManager m_bullets;
    EnemyManager m_enemies;
    Player m_player;
    ParticleSystem m_fx;

    float m_shakeTimer = 0.f;
    float m_shakeMag = 0.f;
    float m_flashTimer = 0.f;
    sf::Color m_flashCol = sf::Color::Transparent;
    float m_bgScrollSpd = 1.f;

    float m_comboTimer = 0.f;
    int m_combo = 0;
    float m_waveTextTimer = 0.f;

    float m_menuPulse = 0.f;
    sf::FloatRect m_playButtonRect{};
    sf::FloatRect m_pauseButtonRect{};

    float m_countdownTimer = 0.f;

    void processEvents();
    void update(float dt);
    void render();

    void startGame();
    void nextWave();
    DamageResult damagePlayer(int amount, sf::Vector2f impact);
    void triggerShake(float mag = Cfg::SHAKE_MAG, float dur = Cfg::SHAKE_DUR);
    void triggerFlash(sf::Color col, float dur = Cfg::FLASH_DUR);

    void drawHUD();
    void drawPauseButton();
    void drawMenu();
    void drawPause();
    void drawGameOver();
    void drawWin();
    void drawLives();
    void drawPowerupHUD();
    void drawCountdown();

    void drawActionRow(float cx, float y,
                       const std::vector<std::string> &keys,
                       const std::string &desc,
                       unsigned descFontSize = 17u);

    std::optional<DamageResult> collide(Bullet &b, Damageable &target, float radius);
    bool hitShields(Bullet &b);

    void checkPlayerBulletVsShields();
    void checkEnemyBulletVsPlayer();
    void checkEnemyBulletVsShields();
    void checkPowerupVsPlayer();
    void checkEnemyVsPlayer();

    void loadHighScore();
    void saveHighScore();

    // Ranking online: a rede roda em outra thread para a janela nao travar.
    std::string m_playerName;
    std::future<std::vector<ScoreEntry>> m_rankingTask;
    std::vector<ScoreEntry> m_ranking;
    std::string m_rankingStatus;
    ScoreEntry m_highlight{"", -1};

    void openRanking(std::optional<ScoreEntry> submit);
    void pollRanking();
    void drawNameEntry();
    void drawRanking();
};