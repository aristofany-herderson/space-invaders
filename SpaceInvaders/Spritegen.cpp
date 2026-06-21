#include "SpriteGen.h"
#include "Constants.h"
#include <cmath>
#include <iostream>

TextureBank GFX;

static void fillPixel(sf::Image &img, int x, int y, sf::Color c)
{
    if (x >= 0 && y >= 0 &&
        x < static_cast<int>(img.getSize().x) &&
        y < static_cast<int>(img.getSize().y))
        img.setPixel({static_cast<unsigned>(x), static_cast<unsigned>(y)}, c);
}

static void fillRect(sf::Image &img, int x, int y, int w, int h, sf::Color c)
{
    for (int dy = 0; dy < h; dy++)
        for (int dx = 0; dx < w; dx++)
            fillPixel(img, x + dx, y + dy, c);
}

static void drawCircle(sf::Image &img, int cx, int cy, int r, sf::Color c, bool filled = true)
{
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
            if (dx * dx + dy * dy <= r * r)
                fillPixel(img, cx + dx, cy + dy, c);
    if (!filled)
    {
        int ir = r - 1;
        for (int dy = -ir; dy <= ir; dy++)
            for (int dx = -ir; dx <= ir; dx++)
                if (dx * dx + dy * dy <= ir * ir)
                    fillPixel(img, cx + dx, cy + dy, sf::Color::Transparent);
    }
}

static sf::Texture fromImage(sf::Image &img)
{
    sf::Texture tex;
    tex.loadFromImage(img);
    tex.setSmooth(false);
    return tex;
}

static void addGlow(sf::Image &img, sf::Color glowCol, int radius = 2)
{
    sf::Image copy = img;
    sf::Vector2u sz = img.getSize();
    for (unsigned y = 0; y < sz.y; ++y)
    {
        for (unsigned x = 0; x < sz.x; ++x)
        {
            if (copy.getPixel({x, y}).a > 0)
            {
                for (int dy = -radius; dy <= radius; ++dy)
                {
                    for (int dx = -radius; dx <= radius; ++dx)
                    {
                        int nx = static_cast<int>(x) + dx;
                        int ny = static_cast<int>(y) + dy;
                        if (nx < 0 || ny < 0 ||
                            nx >= static_cast<int>(sz.x) ||
                            ny >= static_cast<int>(sz.y))
                            continue;
                        sf::Color cur = img.getPixel({(unsigned)nx, (unsigned)ny});
                        if (cur.a == 0)
                        {
                            float dist = std::sqrt(float(dx * dx + dy * dy));
                            float alpha = std::max(0.f, 1.f - dist / (radius + 1));
                            sf::Color glow = glowCol;
                            glow.a = static_cast<std::uint8_t>(80 * alpha);
                            img.setPixel({(unsigned)nx, (unsigned)ny}, glow);
                        }
                    }
                }
            }
        }
    }
}

bool TextureBank::tryLoad(sf::Texture &tex, const std::string &path)
{
    if (tex.loadFromFile(path))
    {
        tex.setSmooth(false);
        return true;
    }
    std::cerr << "[SpriteGen] Nao encontrado: " << path << " - usando fallback procedural\n";
    return false;
}

sf::Texture TextureBank::makePlayer()
{
    const int W = 48, H = 40;
    sf::Image img({(unsigned)W, (unsigned)H}, sf::Color::Transparent);
    sf::Color base = Cfg::COL_PLAYER;
    sf::Color dark{20, 80, 120, 255};
    sf::Color hi{200, 245, 255, 255};
    drawCircle(img, W / 2, 10, 5, hi);
    drawCircle(img, W / 2, 10, 3, sf::Color{120, 200, 255, 255});
    for (int row = 8; row <= 28; ++row)
    {
        float t = float(row - 8) / 20.f;
        int half = static_cast<int>(2.f + t * 14.f);
        for (int dx = -half; dx <= half; ++dx)
        {
            sf::Color c = base;
            if (std::abs(dx) >= half - 1)
                c = hi;
            if (std::abs(dx) <= 1)
                c = dark;
            fillPixel(img, W / 2 + dx, row, c);
        }
    }
    for (int row = 20; row <= 32; ++row)
    {
        float t = float(row - 20) / 12.f;
        int span = static_cast<int>(12 + t * 4);
        for (int dx = span; dx >= 0; --dx)
        {
            sf::Color c = (dx == span) ? hi : base;
            fillPixel(img, W / 2 - 14 - dx, row, c);
            fillPixel(img, W / 2 + 14 + dx, row, c);
        }
    }
    fillRect(img, W / 2 - 5, 29, 4, 7, dark);
    fillRect(img, W / 2 + 1, 29, 4, 7, dark);
    fillRect(img, W / 2 - 18, 30, 4, 6, dark);
    fillRect(img, W / 2 + 14, 30, 4, 6, dark);
    addGlow(img, Cfg::COL_PLAYER, 3);
    return fromImage(img);
}

sf::Texture TextureBank::makeBullet(sf::Color col, float w, float h)
{
    sf::Image img({(unsigned)w, (unsigned)h}, sf::Color::Transparent);
    sf::Color core{255, 255, 255, 255};
    for (int y = 0; y < (int)h; ++y)
    {
        for (int x = 0; x < (int)w; ++x)
        {
            float cx = (w - 1) / 2.f;
            float dist = std::abs(x - cx);
            if (dist < 0.6f)
                fillPixel(img, x, y, core);
            else if (dist < 1.5f)
                fillPixel(img, x, y, col);
            else
                fillPixel(img, x, y, {col.r, col.g, col.b, 100});
        }
    }
    addGlow(img, col, 2);
    return fromImage(img);
}

sf::Texture TextureBank::makeEnemy(EnemyType t)
{
    int W = 36, H = 28;
    sf::Color base, hi, eye;
    switch (t)
    {
    case EnemyType::Drone:
        base = Cfg::COL_DRONE;
        hi = {255, 180, 180, 255};
        eye = {255, 255, 100, 255};
        W = 30;
        H = 24;
        break;
    case EnemyType::Invader:
        base = Cfg::COL_INVADER;
        hi = {180, 255, 180, 255};
        eye = {255, 255, 100, 255};
        W = 34;
        H = 28;
        break;
    case EnemyType::Brute:
        base = Cfg::COL_BRUTE;
        hi = {180, 200, 255, 255};
        eye = {255, 100, 100, 255};
        W = 42;
        H = 34;
        break;
    case EnemyType::Kamikaze:
        base = Cfg::COL_KAMIKAZE;
        hi = {255, 220, 180, 255};
        eye = {255, 80, 80, 255};
        W = 32;
        H = 30;
        break;
    case EnemyType::Elite:
        base = Cfg::COL_ELITE;
        hi = {220, 180, 255, 255};
        eye = {200, 255, 100, 255};
        W = 38;
        H = 32;
        break;
    case EnemyType::UFO:
        base = Cfg::COL_UFO;
        hi = {255, 240, 180, 255};
        eye = {100, 255, 200, 255};
        W = 52;
        H = 22;
        break;
    }
    sf::Image img({(unsigned)W, (unsigned)H}, sf::Color::Transparent);
    if (t == EnemyType::UFO)
    {
        drawCircle(img, W / 2, H / 2 + 2, W / 2 - 2, base);
        drawCircle(img, W / 2, H / 2 - 2, W / 4, hi);
        for (int i = 0; i < 5; ++i)
            drawCircle(img, W / 2 - 16 + i * 8, H / 2 + 4, 2, eye);
    }
    else
    {
        drawCircle(img, W / 2, H / 2, W / 2 - 4, base);
        for (int x = W / 2 - 6; x <= W / 2 + 6; ++x)
            for (int y = H / 2 - 8; y <= H / 2 - 2; ++y)
                fillPixel(img, x, y, hi);
        drawCircle(img, W / 2 - 5, H / 2 - 1, 3, eye);
        drawCircle(img, W / 2 + 5, H / 2 - 1, 3, eye);
        drawCircle(img, W / 2 - 5, H / 2 - 1, 1, {20, 20, 20, 255});
        drawCircle(img, W / 2 + 5, H / 2 - 1, 1, {20, 20, 20, 255});
        if (t == EnemyType::Drone)
        {
            fillRect(img, W / 2 - 5, 0, 2, 4, base);
            fillRect(img, W / 2 + 3, 0, 2, 4, base);
        }
        else if (t == EnemyType::Brute)
        {
            fillRect(img, 0, H / 2 - 4, 7, 10, base);
            fillRect(img, W - 7, H / 2 - 4, 7, 10, base);
            drawCircle(img, 4, H / 2 + 1, 4, hi);
            drawCircle(img, W - 5, H / 2 + 1, 4, hi);
        }
        else if (t == EnemyType::Kamikaze)
        {
            for (int s = 0; s < 3; ++s)
                fillRect(img, W / 2 - 1, H - 4 - s * 4, 3, 3, hi);
        }
        else if (t == EnemyType::Elite)
        {
            for (int s = -1; s <= 1; ++s)
                fillRect(img, W / 2 + s * 8 - 1, 0, 3, 5, base);
            fillRect(img, W / 2 - 1, 0, 3, 6, hi);
        }
    }
    addGlow(img, base, 3);
    return fromImage(img);
}

sf::Texture TextureBank::makeShieldTile(int damage)
{
    const int S = static_cast<int>(Cfg::SHIELD_BLOCK);
    sf::Image img({(unsigned)S, (unsigned)S}, sf::Color::Transparent);
    sf::Color col;
    switch (damage)
    {
    case 0:
        col = {0, 255, 100, 240};
        break;
    case 1:
        col = {160, 255, 50, 200};
        break;
    case 2:
        col = {255, 200, 20, 160};
        break;
    default:
        col = {255, 80, 20, 100};
        break;
    }
    fillRect(img, 0, 0, S, S, col);
    if (damage > 0)
    {
        for (int i = 0; i < damage * 2; ++i)
        {
            int cx = (i * 3 + 1) % (S - 1);
            int cy = (i * 5 + 2) % (S - 1);
            fillPixel(img, cx, cy, sf::Color::Transparent);
        }
    }
    return fromImage(img);
}

sf::Texture TextureBank::makeExplosionFrame(int frame)
{
    const int S = 48;
    sf::Image img({(unsigned)S, (unsigned)S}, sf::Color::Transparent);
    float t = float(frame) / 3.f;
    int r = static_cast<int>(4 + t * 18);
    sf::Color core{255, 255, 200, static_cast<std::uint8_t>(255 * (1.f - t * 0.6f))};
    sf::Color mid{255, 160, 20, static_cast<std::uint8_t>(200 * (1.f - t))};
    sf::Color outer{200, 40, 10, static_cast<std::uint8_t>(150 * (1.f - t))};
    drawCircle(img, S / 2, S / 2, r + 4, outer);
    drawCircle(img, S / 2, S / 2, r, mid);
    drawCircle(img, S / 2, S / 2, r / 2, core);
    addGlow(img, {255, 100, 20, 255}, 4);
    return fromImage(img);
}

sf::Texture TextureBank::makePowerup(sf::Color col, const std::string &)
{
    const int S = 20;
    sf::Image img({(unsigned)S, (unsigned)S}, sf::Color::Transparent);
    drawCircle(img, S / 2, S / 2, S / 2 - 1, col);
    sf::Color inner{
        static_cast<std::uint8_t>(col.r / 3),
        static_cast<std::uint8_t>(col.g / 3),
        static_cast<std::uint8_t>(col.b / 3), 200};
    drawCircle(img, S / 2, S / 2, S / 2 - 4, inner);
    addGlow(img, col, 3);
    return fromImage(img);
}

void TextureBank::generate()
{
    const std::string dir = "assets/sprites/";

    if (!tryLoad(player, dir + "player.png"))
        player = makePlayer();

    tryLoad(star, dir + "star.png");

    if (!tryLoad(bulletPlayer, dir + "bullet_player.png"))
        bulletPlayer = makeBullet({80, 220, 255, 255}, 5, 18);
    if (!tryLoad(bulletEnemy, dir + "bullet_enemy.png"))
        bulletEnemy = makeBullet({255, 80, 80, 255}, 5, 18);
    if (!tryLoad(bulletEnemyBase, dir + "bullet_enemy_base.png"))
        bulletEnemyBase = makeBullet({255, 255, 255, 255}, 5, 18);

    static const char *enemyFiles[] = {
        "enemy_drone.png", "enemy_invader.png", "enemy_brute.png",
        "enemy_kamikaze.png", "enemy_elite.png", "enemy_ufo.png"};
    for (int i = 0; i < 6; ++i)
    {
        if (!tryLoad(enemy[i], dir + enemyFiles[i]))
            enemy[i] = makeEnemy(static_cast<EnemyType>(i));
    }
    for (int i = 0; i < 4; ++i)
    {
        std::string path = dir + "shield_" + std::to_string(i) + ".png";
        if (!tryLoad(shield[i], path))
            shield[i] = makeShieldTile(i);
    }

    for (int i = 0; i < 4; ++i)
    {
        std::string path = dir + "explosion_" + std::to_string(i) + ".png";
        if (!tryLoad(explosion[i], path))
            explosion[i] = makeExplosionFrame(i);
    }

    if (!tryLoad(powerupRapid, dir + "powerup_rapid.png"))
        powerupRapid = makePowerup({255, 200, 20, 255}, "R");
    if (!tryLoad(powerupTriple, dir + "powerup_triple.png"))
        powerupTriple = makePowerup({80, 255, 200, 255}, "T");
    if (!tryLoad(powerupShield, dir + "powerup_shield.png"))
        powerupShield = makePowerup({100, 150, 255, 255}, "S");

    tryLoad(volumeIcon, dir + "volume.png");
    tryLoad(pauseIcon, dir + "pause.png");
}