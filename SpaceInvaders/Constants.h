#pragma once
#include <SFML/Graphics.hpp>

namespace Cfg
{
    // Window
    constexpr unsigned W = 900, H = 700;
    constexpr const char *TITLE = "SpaceInvaders";

    // Player
    constexpr int MAX_WAVES = 5;
    constexpr float PLAYER_SPEED = 230.f;
    constexpr float PLAYER_SHOOT_CD = 0.22f;
    constexpr float PLAYER_INVUL = 2.5f;
    constexpr float PLAYER_BLINK = 0.09f;
    constexpr int PLAYER_LIVES = 3;
    constexpr float PLAYER_BULLET_SPD = 520.f;

    // Enemy
    constexpr float ENEMY_BULLET_SPD = 240.f;
    constexpr float ENEMY_SHOOT_MIN = 8.0f;
    constexpr float ENEMY_SHOOT_MAX = 16.0f;
    constexpr float ENEMY_SHOOT_WAVE_SCL = 0.35f;
    constexpr int FORMATION_COLS = 11;
    constexpr int FORMATION_ROWS = 5;
    constexpr float FORM_START_X = 80.f;
    constexpr float FORM_START_Y = 90.f;
    constexpr float FORM_SPACING_X = 68.f;
    constexpr float FORM_SPACING_Y = 58.f;
    constexpr float FORM_STEP_H = 340.f;
    constexpr float FORM_STEP_V = 24.f;
    constexpr float FORM_SPEED_BASE = 38.f;
    constexpr float FORM_SPEED_SCALE = 2.2f;

    // UFO
    constexpr float UFO_SPEED = 140.f;
    constexpr float UFO_INTERVAL_MIN = 18.f;
    constexpr float UFO_INTERVAL_MAX = 35.f;
    constexpr int UFO_SCORE = 500;

    // Scores
    constexpr int SCORE_DRONE = 10;
    constexpr int SCORE_INVADER = 20;
    constexpr int SCORE_BRUTE = 40;
    constexpr int SCORE_KAMIKAZE = 60;
    constexpr int SCORE_ELITE = 100;

    // Shields
    constexpr int SHIELD_COLS = 10;
    constexpr int SHIELD_ROWS = 7;
    constexpr float SHIELD_BLOCK = 7.f;
    constexpr int SHIELD_COUNT = 4;

    // Effects
    constexpr int EXPLOSION_PARTS = 38;
    constexpr float PART_LIFETIME = 1.1f;
    constexpr float SHAKE_DUR = 0.45f;
    constexpr float SHAKE_MAG = 9.f;
    constexpr float FLASH_DUR = 0.18f;

    // HUD
    constexpr float HUD_HEIGHT = 48.f;

    // Power-ups
    constexpr float POWERUP_DROP_CHANCE = 0.12f;
    constexpr float POWERUP_SPEED = 90.f;
    constexpr float POWERUP_DURATION = 8.f;

    // Lives bonus
    constexpr int LIFE_BONUS_PER_WAVE = 1;
    constexpr float LIFE_NOTIF_DURATION = 2.2f;
    constexpr float LIFE_NOTIF_RISE = 45.f;

    // Kamikaze
    constexpr float KAMIKAZE_DIVE_BASE = 0.006f;
    constexpr float KAMIKAZE_DIVE_WAVE = 0.003f;

    // Colors
    inline sf::Color COL_DRONE{255, 60, 60, 255};
    inline sf::Color COL_INVADER{60, 255, 80, 255};
    inline sf::Color COL_BRUTE{60, 140, 255, 255};
    inline sf::Color COL_KAMIKAZE{255, 160, 20, 255};
    inline sf::Color COL_ELITE{200, 60, 255, 255};
    inline sf::Color COL_UFO{255, 220, 40, 255};
    inline sf::Color COL_PLAYER{80, 220, 255, 255};
}
