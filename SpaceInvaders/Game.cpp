#include "Game.h"
#include "Constants.h"
#include "RoundedRectShape.h"
#include <fstream>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

Game::Game()
    : m_window(sf::VideoMode({Cfg::W, Cfg::H}), Cfg::TITLE,
               sf::Style::Titlebar | sf::Style::Close)
{
    m_window.setVerticalSyncEnabled(true);

    GFX.generate();
    SFX.loadAll();
    SFX.startMusic("assets/sounds/bg_music.wav", 80.f);

    sf::Image icon;
    if (icon.loadFromFile("assets/icon.png"))
        m_window.setIcon(icon);

    if (!m_font.openFromFile("assets/fonts/font.ttf"))
        throw std::runtime_error("Nao foi possivel carregar assets/fonts/font.ttf");

    loadHighScore();
    loadSettings();
}

void Game::loadSettings()
{
    std::ifstream f("settings.dat");
    if (f.is_open())
    {
        f >> m_volume;
        m_volume = std::clamp(m_volume, 0.f, 100.f);
        SFX.setMasterVolume(m_volume);
    }
}

void Game::saveSettings()
{
    std::ofstream f("settings.dat");
    if (f.is_open())
        f << m_volume;
}

void Game::loadHighScore()
{
    std::ifstream f("hiscore.dat");
    if (f.is_open())
        f >> m_hiScore;
}

void Game::saveHighScore()
{
    if (m_player.getScore() > m_hiScore)
    {
        m_hiScore = m_player.getScore();
        std::ofstream f("hiscore.dat");
        f << m_hiScore;
    }
}

void Game::drawVolumeSlider(float cx, float cy, float width)
{
    const float iconSz = 18.f;
    const float trackH = 4.f;
    const float knobR = 6.f;
    const float pad = 8.f;
    const float lblW = 34.f;

    float totalW = iconSz + pad + width + pad + lblW;
    float startX = cx - totalW / 2.f;
    float iconX = startX;
    float trackX = startX + iconSz + pad;
    float trackY = cy - trackH / 2.f;
    float knobX = trackX + width * (m_volume / 100.f);
    float knobY = cy;

    sf::Sprite icon(GFX.volumeIcon);
    auto isz = GFX.volumeIcon.getSize();
    float sc = iconSz / float(std::max(isz.x, isz.y));
    icon.setScale({sc, sc});
    icon.setOrigin({isz.x / 2.f, isz.y / 2.f});
    icon.setPosition({iconX + iconSz / 2.f, cy});
    m_window.draw(icon);

    RoundedRectShape track({width, trackH}, trackH / 2.f);
    track.setPosition({trackX, trackY});
    track.setFillColor({25, 25, 55, 210});
    track.setOutlineColor({60, 90, 170, 160});
    track.setOutlineThickness(1.f);
    m_window.draw(track);

    float fillW = width * (m_volume / 100.f);
    if (fillW > trackH)
    {
        RoundedRectShape fill({fillW, trackH},
                              std::min(trackH / 2.f, fillW / 2.f));
        fill.setPosition({trackX, trackY});
        fill.setFillColor({70, 170, 255, 220});
        m_window.draw(fill);
    }

    sf::CircleShape knob(knobR);
    knob.setOrigin({knobR, knobR});
    knob.setPosition({knobX, knobY});
    knob.setFillColor({210, 235, 255, 255});
    knob.setOutlineColor({80, 150, 255, 255});
    knob.setOutlineThickness(1.5f);
    m_window.draw(knob);

    sf::Text pct(m_font,
                 std::to_string(static_cast<int>(m_volume)) + "%", 12u);
    pct.setFillColor({150, 185, 225, 200});
    auto pb = pct.getLocalBounds();
    pct.setPosition({trackX + width + pad, cy - pb.size.y / 2.f - pb.position.y});
    m_window.draw(pct);

    m_volTrackRect = {{trackX - knobR, cy - knobR - 2.f},
                      {width + knobR * 2.f, knobR * 2.f + 4.f}};
}

void Game::startGame()
{
    m_wave = 1;
    m_combo = 0;
    m_flashTimer = 0.f;
    m_shakeTimer = 0.f;

    m_player.init();
    m_shields.init();
    m_bullets.killAll();
    m_fx.clear();
    m_enemies.spawnFormation(m_wave);

    m_state = GameState::Countdown;
    m_countdownTimer = 4.0f;
    m_bgScrollSpd = 1.f;

    SFX.play(SoundId::WaveStart, 85.f);
}

void Game::nextWave()
{
    m_enemies.stopUFOLoop();

    m_wave++;
    m_shields.reset();
    m_bullets.killAll();
    m_fx.clear();
    m_enemies.spawnFormation(m_wave);

    m_bgScrollSpd = 1.f + m_wave * 0.15f;
    triggerFlash({80, 180, 255, 180}, 0.4f);
    SFX.play(SoundId::WaveStart, 85.f);
    m_state = GameState::Countdown;
    m_countdownTimer = 4.0f;

    m_player.addLife();
    SFX.play(SoundId::Powerup, 90.f);
}

void Game::triggerShake(float mag, float dur)
{
    m_shakeMag = mag;
    m_shakeTimer = dur;
}

void Game::triggerFlash(sf::Color col, float dur)
{
    m_flashCol = col;
    m_flashTimer = dur;
}

void Game::run()
{
    while (m_window.isOpen())
    {
        float dt = m_clock.restart().asSeconds();
        dt = std::min(dt, 0.05f);

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents()
{
    while (const auto ev = m_window.pollEvent())
    {
        if (ev->is<sf::Event::Closed>())
        {
            m_window.close();
            return;
        }

        if (const auto *kp = ev->getIf<sf::Event::KeyPressed>())
        {
            switch (m_state)
            {
            case GameState::Menu:
                if (kp->code == sf::Keyboard::Key::Enter ||
                    kp->code == sf::Keyboard::Key::Space)
                    startGame();
                if (kp->code == sf::Keyboard::Key::Escape)
                    m_window.close();
                break;
            case GameState::Countdown:

                break;
            case GameState::Playing:
                if (kp->code == sf::Keyboard::Key::Escape)
                    m_state = GameState::Paused;
                break;
            case GameState::Paused:
                if (kp->code == sf::Keyboard::Key::Escape ||
                    kp->code == sf::Keyboard::Key::P)
                    m_state = GameState::Playing;
                if (kp->code == sf::Keyboard::Key::M)
                    m_state = GameState::Menu;
                break;
            case GameState::GameOver:
            case GameState::Win:
                if (kp->code == sf::Keyboard::Key::Enter ||
                    kp->code == sf::Keyboard::Key::Space)
                    m_state = GameState::Menu;
                if (kp->code == sf::Keyboard::Key::R)
                    startGame();
                break;
            }
        }

        if (m_state == GameState::Menu)
        {
            if (const auto *mb = ev->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mb->button == sf::Mouse::Button::Left)
                {
                    sf::Vector2f mp{float(mb->position.x), float(mb->position.y)};
                    if (m_playButtonRect.contains(mp))
                        startGame();
                }
            }
        }

        if (m_state == GameState::Menu || m_state == GameState::Paused)
        {
            if (const auto *mb = ev->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mb->button == sf::Mouse::Button::Left)
                {
                    sf::Vector2f mp{float(mb->position.x), float(mb->position.y)};
                    if (m_volTrackRect.contains(mp))
                    {
                        m_volumeDragging = true;

                        float ratio = std::clamp(
                            (mp.x - m_volTrackRect.position.x) / m_volTrackRect.size.x,
                            0.f, 1.f);
                        m_volume = ratio * 100.f;
                        SFX.setMasterVolume(m_volume);
                    }
                }
            }

            if (const auto *mr = ev->getIf<sf::Event::MouseButtonReleased>())
            {
                if (mr->button == sf::Mouse::Button::Left && m_volumeDragging)
                {
                    m_volumeDragging = false;
                    saveSettings();
                }
            }

            if (const auto *mm = ev->getIf<sf::Event::MouseMoved>())
            {
                if (m_volumeDragging)
                {
                    sf::Vector2f mp{float(mm->position.x), float(mm->position.y)};
                    float ratio = std::clamp(
                        (mp.x - m_volTrackRect.position.x) / m_volTrackRect.size.x,
                        0.f, 1.f);
                    m_volume = ratio * 100.f;
                    SFX.setMasterVolume(m_volume);
                }
            }
        }
    }
}

void Game::update(float dt)
{
    m_menuPulse += dt;

    if (m_shakeTimer > 0.f)
        m_shakeTimer -= dt;
    if (m_flashTimer > 0.f)
        m_flashTimer -= dt;

    if (m_state == GameState::Countdown)
    {
        m_bg.update(dt);
        m_countdownTimer -= dt;
        if (m_countdownTimer <= 0.f)
        {
            m_state = GameState::Playing;
            m_waveTextTimer = 2.5f;
        }
        return;
    }

    if (m_state != GameState::Playing)
    {
        m_bg.update(dt);
        return;
    }

    if (m_comboTimer > 0.f)
        m_comboTimer -= dt;
    else
        m_combo = 0;
    if (m_waveTextTimer > 0.f)
        m_waveTextTimer -= dt;

    float shakeX = 0.f, shakeY = 0.f;
    if (m_shakeTimer > 0.f)
    {
        float t = m_shakeTimer / Cfg::SHAKE_DUR;
        shakeX = (std::rand() % 2 ? 1.f : -1.f) * m_shakeMag * t;
        shakeY = (std::rand() % 2 ? 1.f : -1.f) * m_shakeMag * t;
    }

    m_bg.update(dt, shakeX, shakeY);
    m_bg.setScrollSpeed(m_bgScrollSpd);

    m_player.update(dt, m_bullets, m_fx);

    m_enemies.update(dt, m_player.getPos(), m_bullets, m_fx, [this](sf::Vector2f pos, sf::Color col, int score)
                     {
            if (score > 0) {
                m_comboTimer = 1.2f;
                m_combo++;
                int finalScore = score * std::max(1, m_combo / 3);
                m_player.addScore(finalScore);
                m_fx.addFloatingScore(pos, finalScore, col);
            }
            SFX.play(SoundId::Explosion, 75.f, 0.9f + (std::rand() % 200) / 1000.f);
            triggerShake(3.f, 0.15f); }, [this](sf::Vector2f pos)
                     {
                    if (m_player.hasShield()) {
                        triggerShake(4.f, 0.2f);
                        triggerFlash({ 100, 180, 255, 120 }, 0.25f);
                        SFX.play(SoundId::ShieldHit, 80.f);
                        return;
                    }
                    auto result = m_player.takeDamage(m_fx);
                    if (result != DamageResult::Ignored) {
                        triggerShake(10.f, 0.4f);
                        triggerFlash({ 255, 80, 20, 200 }, 0.5f);
                        SFX.play(SoundId::Explosion, 90.f, 0.7f);
                    }
                    if (result == DamageResult::Dead) {
                        saveHighScore();
                        m_state = GameState::GameOver;
                        SFX.play(SoundId::GameOver, 90.f);
                        m_enemies.stopUFOLoop();
                        triggerShake(20.f, 1.f);
                        triggerFlash({ 255, 30, 30, 200 }, 0.6f);
                    } });

    m_bullets.update(dt);

    checkPlayerBulletVsShields();
    checkEnemyBulletVsShields();
    checkEnemyBulletVsPlayer();
    checkPowerupVsPlayer();
    checkEnemyVsPlayer();

    m_fx.update(dt);

    sf::View view = m_window.getDefaultView();
    view.setCenter({float(Cfg::W) / 2.f + shakeX, float(Cfg::H) / 2.f + shakeY});
    m_window.setView(view);

    if (!m_player.isAlive())
    {
        saveHighScore();
        m_state = GameState::GameOver;
        SFX.play(SoundId::GameOver, 90.f);
        m_enemies.stopUFOLoop();
        triggerShake(20.f, 1.f);
        triggerFlash({255, 30, 30, 200}, 0.6f);
    }
    if (m_enemies.allDead())
    {
        if (m_wave >= Cfg::MAX_WAVES)
        {
            saveHighScore();
            m_state = GameState::Win;
            m_enemies.stopUFOLoop();
            SFX.play(SoundId::Win, 90.f);
        }
        else
        {
            nextWave();
        }
    }
    if (m_enemies.hasReachedBottom())
    {
        auto result = m_player.takeDamage(m_fx);
        if (result != DamageResult::Ignored)
        {
            triggerShake();
            triggerFlash({255, 60, 60, 180});
        }
        if (result == DamageResult::Dead)
        {
            saveHighScore();
            m_state = GameState::GameOver;
            SFX.play(SoundId::GameOver, 90.f);
            m_enemies.stopUFOLoop();
            triggerShake(20.f, 1.f);
            triggerFlash({255, 30, 30, 200}, 0.6f);
        }
    }
}

void Game::checkPlayerBulletVsShields()
{
    for (auto &b : m_bullets.getAll())
    {
        if (b.owner != BulletOwner::Player || !b.alive)
            continue;
        if (m_shields.checkBulletHit(b.pos))
        {
            b.alive = false;
            SFX.play(SoundId::ShieldHit, 50.f);
        }
    }
}

void Game::checkEnemyBulletVsShields()
{
    for (auto &b : m_bullets.getAll())
    {
        if (b.owner != BulletOwner::Enemy || !b.alive)
            continue;
        if (m_shields.checkBulletHit(b.pos))
        {
            b.alive = false;
            SFX.play(SoundId::ShieldHit, 60.f);
        }
    }
}

void Game::checkEnemyBulletVsPlayer()
{
    sf::FloatRect pb = m_player.getBounds();
    for (auto &b : m_bullets.getAll())
    {
        if (b.owner != BulletOwner::Enemy || !b.alive)
            continue;
        if (pb.contains(b.pos))
        {
            b.alive = false;
            auto result = m_player.takeDamage(m_fx);
            if (result != DamageResult::Ignored)
            {
                triggerShake();
                triggerFlash({255, 50, 50, 160});
            }
            if (result == DamageResult::Dead)
            {
                saveHighScore();
                m_state = GameState::GameOver;
            }
        }
    }
}

void Game::checkPowerupVsPlayer()
{
    sf::FloatRect pb = m_player.getBounds();
    pb = {
        {pb.position.x - 10.f, pb.position.y - 10.f},
        {pb.size.x + 20.f, pb.size.y + 20.f}};
    for (auto &p : m_enemies.getPowerups())
    {
        if (!p.alive)
            continue;
        sf::FloatRect pr{p.pos - sf::Vector2f{10.f, 10.f}, {20.f, 20.f}};
        if (pb.findIntersection(pr))
        {
            p.alive = false;
            m_player.applyPowerup(p.type);
            triggerFlash({100, 200, 255, 80}, 0.2f);
        }
    }
}

void Game::checkEnemyVsPlayer()
{
    sf::FloatRect pb = m_player.getBounds();
    (void)pb;
}

void Game::render()
{
    m_window.clear({4, 4, 18, 255});
    m_bg.draw(m_window);

    switch (m_state)
    {
    case GameState::Menu:
        drawMenu();
        break;

    case GameState::Countdown:
        m_shields.draw(m_window);
        // m_enemies.draw(m_window);
        // m_enemies.drawUFO(m_window);
        m_player.draw(m_window);
        m_fx.draw(m_window, m_font);
        drawHUD();
        drawCountdown();
        break;

    case GameState::Playing:
    case GameState::Paused:
        m_shields.draw(m_window);
        m_enemies.draw(m_window);
        m_enemies.drawUFO(m_window);
        m_bullets.draw(m_window);
        m_player.draw(m_window);
        m_fx.draw(m_window, m_font);
        drawHUD();
        if (m_state == GameState::Paused)
            drawPause();
        break;

    case GameState::GameOver:
        m_shields.draw(m_window);
        m_fx.draw(m_window, m_font);
        drawGameOver();
        drawHUD();
        break;

    case GameState::Win:
        drawWin();
        drawHUD();
        break;
    }

    if (m_flashTimer > 0.f)
    {
        float alpha = (m_flashTimer / Cfg::FLASH_DUR);
        sf::RectangleShape flash({float(Cfg::W), float(Cfg::H)});
        sf::Color fc = m_flashCol;
        fc.a = static_cast<std::uint8_t>(fc.a * alpha);
        flash.setFillColor(fc);
        m_window.setView(m_window.getDefaultView());
        m_window.draw(flash);
    }

    m_window.display();
    m_window.setView(m_window.getDefaultView());
}

void Game::drawActionRow(float cx, float y,
                         const std::vector<std::string> &keys,
                         const std::string &desc,
                         unsigned descFontSize)
{
    const float badgeH = 34.f;
    const float cornerRadius = 6.f;
    const float padX = 13.f;
    const float keyGap = 6.f;
    const float descGap = 16.f;
    const unsigned keyFont = 14u;

    std::vector<float> widths;
    float totalKeyW = 0.f;
    for (std::size_t ki = 0; ki < keys.size(); ++ki)
    {
        sf::Text kt(m_font, keys[ki], keyFont);
        float tw = kt.getLocalBounds().size.x;
        float w = std::max(badgeH, tw + padX * 2.f);
        widths.push_back(w);
        totalKeyW += w + (ki > 0 ? keyGap : 0.f);
    }

    sf::Text descTxt(m_font, desc, descFontSize);
    auto db = descTxt.getLocalBounds();
    float descW = db.size.x;

    float rowW = totalKeyW + descGap + descW;
    float startX = cx - rowW / 2.f;

    float x = startX;
    for (std::size_t ki = 0; ki < keys.size(); ++ki)
    {
        if (ki > 0)
            x += keyGap;
        float kw = widths[ki];

        RoundedRectShape badge({kw, badgeH}, cornerRadius);
        badge.setPosition({x, y});
        badge.setFillColor({30, 50, 100, 200});
        badge.setOutlineColor({100, 160, 255, 220});
        badge.setOutlineThickness(1.5f);
        m_window.draw(badge);

        RoundedRectShape shine({kw - 4.f, 5.f}, 3.f, 4u);
        shine.setPosition({x + 2.f, y + 2.f});
        shine.setFillColor({255, 255, 255, 28});
        m_window.draw(shine);

        sf::RectangleShape shadow({kw, 3.f});
        shadow.setPosition({x, y + badgeH + 1.f});
        shadow.setFillColor({0, 0, 0, 55});
        m_window.draw(shadow);

        sf::Text kt(m_font, keys[ki], keyFont);
        kt.setFillColor({200, 230, 255, 255});
        auto kb = kt.getLocalBounds();
        kt.setOrigin({kb.position.x + kb.size.x / 2.f,
                      kb.position.y + kb.size.y / 2.f});
        kt.setPosition({x + kw / 2.f, y + badgeH / 2.f});
        m_window.draw(kt);

        x += kw;
    }

    descTxt.setFillColor({180, 200, 255, 210});
    descTxt.setOrigin({0.f, db.position.y + db.size.y / 2.f});
    descTxt.setPosition({startX + totalKeyW + descGap, y + badgeH / 2.f});
    m_window.draw(descTxt);
}

void Game::drawCountdown()
{
    float phase = 4.0f - m_countdownTimer;
    int step = static_cast<int>(phase);
    float t = phase - static_cast<float>(step);

    std::string label;
    sf::Color col;

    switch (step)
    {
    case 0:
        label = "3";
        col = {255, 60, 60, 255};
        break;
    case 1:
        label = "2";
        col = {255, 220, 50, 255};
        break;
    case 2:
        label = "1";
        col = {60, 255, 100, 255};
        break;
    default:
        label = "GO!";
        col = {80, 220, 255, 255};
        break;
    }

    float popScale = 1.7f - t * 0.7f;

    float alpha = 1.f;
    if (step < 3 && t > 0.75f)
        alpha = 1.f - (t - 0.75f) / 0.25f;
    col.a = static_cast<std::uint8_t>(std::max(0.f, alpha) * 255.f);

    const float cx = float(Cfg::W) / 2.f;
    const float cy = float(Cfg::H) / 2.f - 20.f;

    unsigned fsize = (step == 3) ? 64u : 88u;

    sf::Text txt(m_font, label, fsize);
    txt.setFillColor(col);
    txt.setOutlineColor({0, 0, 0, col.a});
    txt.setOutlineThickness(5.f);

    auto b = txt.getLocalBounds();
    txt.setOrigin({b.position.x + b.size.x / 2.f,
                   b.position.y + b.size.y / 2.f});
    txt.setScale({popScale, popScale});
    txt.setPosition({cx, cy});
    m_window.draw(txt);

    const float barW = 200.f;
    const float barH = 4.f;
    const float barX = cx - barW / 2.f;
    const float barY = cy + 78.f;

    sf::RectangleShape track({barW, barH});
    track.setPosition({barX, barY});
    track.setFillColor({40, 40, 60, 140});
    m_window.draw(track);

    float filled = barW * t;
    if (filled > 0.f)
    {
        sf::Color barCol = col;
        barCol.a = 180;
        sf::RectangleShape fill({filled, barH});
        fill.setPosition({barX, barY});
        fill.setFillColor(barCol);
        m_window.draw(fill);
    }
}

void Game::drawHUD()
{
    sf::RectangleShape bar({float(Cfg::W), Cfg::HUD_HEIGHT});
    bar.setFillColor({10, 10, 30, 210});
    m_window.draw(bar);

    sf::RectangleShape bbar({float(Cfg::W), 2.f});
    bbar.setPosition({0.f, Cfg::HUD_HEIGHT});
    bbar.setFillColor({60, 120, 255, 100});
    m_window.draw(bbar);

    {
        std::ostringstream ss;
        ss << std::setw(8) << std::setfill('0') << m_player.getScore();
        sf::Text t(m_font, "SCORE " + ss.str(), 17u);
        t.setFillColor({80, 220, 255, 255});
        t.setPosition({12.f, 12.f});
        m_window.draw(t);
    }

    {
        int hi = std::max(m_hiScore, m_player.getScore());
        std::ostringstream ss;
        ss << std::setw(8) << std::setfill('0') << hi;
        sf::Text t(m_font, "HI " + ss.str(), 15u);
        t.setFillColor({255, 220, 80, 200});
        auto b = t.getLocalBounds();
        t.setPosition({float(Cfg::W) / 2.f - b.size.x / 2.f, 14.f});
        m_window.draw(t);
    }

    {
        sf::Text t(m_font, "WAVE " + std::to_string(m_wave), 17u);
        t.setFillColor({200, 60, 255, 220});
        auto b = t.getLocalBounds();
        t.setPosition({float(Cfg::W) - b.size.x - 12.f, 12.f});
        m_window.draw(t);
    }

    drawLives();

    if (m_player.hasLifeNotif())
    {
        auto alpha = static_cast<std::uint8_t>(m_player.lifeNotifAlpha());
        sf::Vector2f npos = m_player.lifeNotifPos();

        sf::Sprite lifeIcon(GFX.player);
        auto isz = GFX.player.getSize();
        lifeIcon.setScale({0.38f, 0.38f});
        lifeIcon.setOrigin({isz.x / 2.f, isz.y / 2.f});
        lifeIcon.setColor({255, 255, 255, alpha});
        lifeIcon.setPosition({npos.x - 22.f, npos.y});
        m_window.draw(lifeIcon);

        sf::Text lifeText(m_font, "+1 UP", 18u);
        lifeText.setFillColor({80, 255, 160, alpha});
        lifeText.setOutlineColor({0, 0, 0, alpha});
        lifeText.setOutlineThickness(2.f);
        auto lb = lifeText.getLocalBounds();
        lifeText.setOrigin({lb.size.x / 2.f, lb.size.y / 2.f});
        lifeText.setPosition(npos);
        m_window.draw(lifeText);
    }

    drawPowerupHUD();

    if (m_waveTextTimer > 0.f)
    {
        float alpha = std::min(1.f, m_waveTextTimer) * 255.f;
        std::string wt = (m_wave == 1) ? "WAVE 1" : "WAVE " + std::to_string(m_wave) + "  INCOMING!";
        sf::Text t(m_font, wt, 28u);
        t.setFillColor({255, 255, 80, static_cast<std::uint8_t>(alpha)});
        t.setOutlineColor({0, 0, 0, static_cast<std::uint8_t>(alpha)});
        t.setOutlineThickness(2.f);
        auto b = t.getLocalBounds();
        t.setOrigin({b.size.x / 2.f, b.size.y / 2.f});
        t.setPosition({float(Cfg::W) / 2.f, float(Cfg::H) / 2.f - 60.f});
        m_window.draw(t);
    }

    if (m_combo >= 3 && m_comboTimer > 0.f)
    {
        sf::Text t(m_font, "x" + std::to_string(m_combo) + " COMBO!", 21u);
        t.setFillColor({255, 200, 50, 220});
        auto b = t.getLocalBounds();
        t.setOrigin({b.size.x / 2.f, b.size.y / 2.f});
        t.setPosition({float(Cfg::W) / 2.f, Cfg::HUD_HEIGHT + 30.f});
        m_window.draw(t);
    }
}

void Game::drawLives()
{
    sf::Sprite icon(GFX.player);
    auto sz = GFX.player.getSize();
    icon.setScale({0.45f, 0.45f});
    for (int i = 0; i < m_player.getLives(); ++i)
    {
        icon.setPosition({float(Cfg::W) - (i + 1) * (sz.x * 0.45f + 4.f) - 8.f,
                          float(Cfg::H) - sz.y * 0.45f - 8.f});
        m_window.draw(icon);
    }
}

void Game::drawPowerupHUD()
{
    struct PuSlot
    {
        float timer;
        const sf::Texture *tex;
        sf::Color barCol;
        std::string label;
    };

    std::vector<PuSlot> slots;
    if (m_player.getRapidTimer() > 0.f)
        slots.push_back({m_player.getRapidTimer(), &GFX.powerupRapid, {255, 200, 20, 255}, "RAPID"});
    if (m_player.getTripleTimer() > 0.f)
        slots.push_back({m_player.getTripleTimer(), &GFX.powerupTriple, {80, 255, 200, 255}, "TRIPLE"});
    if (m_player.getShieldTimer() > 0.f)
        slots.push_back({m_player.getShieldTimer(), &GFX.powerupShield, {100, 150, 255, 255}, "SHIELD"});

    if (slots.empty())
        return;

    const float iconSz = 22.f;
    const float barW = 60.f;
    const float barH = 6.f;
    const float padX = 10.f;
    const float padY = 8.f;
    const float slotH = iconSz + 14.f;
    const float gap = 6.f;

    float totalHeight = float(slots.size()) * slotH + float(slots.size() - 1) * gap;
    float baseY = float(Cfg::H) - totalHeight - padY - 4.f;

    for (std::size_t i = 0; i < slots.size(); ++i)
    {
        const auto &s = slots[i];
        float y = baseY + float(i) * (slotH + gap);

        RoundedRectShape bg({iconSz + barW + 38.f, slotH - 2.f}, 4.f);
        bg.setPosition({padX - 4.f, y});
        bg.setFillColor({10, 10, 30, 160});
        bg.setOutlineColor({s.barCol.r, s.barCol.g, s.barCol.b, 80});
        bg.setOutlineThickness(1.f);
        m_window.draw(bg);

        sf::Sprite spr(*s.tex);
        auto tsz = s.tex->getSize();
        float scale = iconSz / float(std::max(tsz.x, tsz.y));
        spr.setScale({scale, scale});
        spr.setPosition({padX, y + (slotH - iconSz) / 2.f - 1.f});
        m_window.draw(spr);

        float ratio = std::min(1.f, s.timer / Cfg::POWERUP_DURATION);
        float filledW = barW * ratio;
        float barX = padX + iconSz + 6.f;
        float barY = y + (slotH - barH) / 2.f - 1.f;

        RoundedRectShape track({barW, barH}, barH * 0.5f);
        track.setPosition({barX, barY});
        track.setFillColor({30, 30, 60, 200});
        track.setOutlineColor({60, 60, 100, 150});
        track.setOutlineThickness(1.f);
        m_window.draw(track);

        bool blink = (s.timer < 3.f && std::fmod(s.timer, 0.4f) < 0.2f);
        if (!blink && filledW > 0.f)
        {
            RoundedRectShape fill(
                {filledW, barH},
                std::min(barH * 0.5f, filledW * 0.5f));

            fill.setPosition({barX, barY});

            sf::Color fc = s.barCol;
            fc.a = static_cast<std::uint8_t>(180 + ratio * 75);
            fill.setFillColor(fc);

            m_window.draw(fill);
        }

        int secs = static_cast<int>(std::ceil(s.timer));
        sf::Text timeText(m_font, std::to_string(secs) + "s", 13u);
        timeText.setFillColor(s.timer < 3.f
                                  ? sf::Color{255, 80, 80, 255}
                                  : sf::Color{s.barCol.r, s.barCol.g, s.barCol.b, 220});
        timeText.setPosition({barX + barW + 8.f, barY - 6.f});
        m_window.draw(timeText);
    }
}

void Game::drawMenu()
{
    const float cx = float(Cfg::W) / 2.f;
    float pulse = std::sin(m_menuPulse * 2.2f) * 0.5f + 0.5f;

    {
        sf::Text title(m_font, "SPACE INVADERS", 46u);
        title.setFillColor({80, 220, 255, 255});
        title.setOutlineColor({0, 60, 120, 255});
        title.setOutlineThickness(3.f);
        auto tb = title.getLocalBounds();
        title.setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
        float s = 1.f + pulse * 0.03f;
        title.setScale({s, s});
        title.setPosition({cx, 155.f});
        m_window.draw(title);
    }

    {
        const float btnW = 220.f, btnH = 52.f;
        const float btnY = 268.f;
        m_playButtonRect = {{cx - btnW / 2.f, btnY}, {btnW, btnH}};

        sf::Vector2i mp = sf::Mouse::getPosition(m_window);
        bool hovered = m_playButtonRect.contains(
            sf::Vector2f{float(mp.x), float(mp.y)});

        RoundedRectShape btn({btnW, btnH}, 10.f);
        btn.setPosition({cx - btnW / 2.f, btnY});
        btn.setFillColor(hovered ? sf::Color{40, 160, 255, 220}
                                 : sf::Color{20, 80, 180, 180});
        btn.setOutlineColor(hovered ? sf::Color{120, 220, 255, 255}
                                    : sf::Color{60, 140, 255, 200});
        btn.setOutlineThickness(2.f);
        m_window.draw(btn);

        sf::RectangleShape shine({btnW - 20.f, 6.f});
        shine.setPosition({cx - (btnW - 20.f) / 2.f, btnY + 4.f});
        shine.setFillColor({255, 255, 255, hovered ? 50u : 25u});
        m_window.draw(shine);

        sf::Text playTxt(m_font, "PLAY", 26u);
        playTxt.setFillColor({255, 255, 255, 255});
        playTxt.setOutlineColor({0, 40, 100, 200});
        playTxt.setOutlineThickness(2.f);
        auto pb = playTxt.getLocalBounds();
        playTxt.setOrigin({pb.size.x / 2.f, pb.size.y / 2.f});
        playTxt.setPosition({cx, btnY + btnH / 2.f - 5.f});
        m_window.draw(playTxt);
    }

    {
        const float panelW = 390.f;
        const float panelX = cx - panelW / 2.f;
        const float panelY = 346.f;
        const float BTN_H = 32.f;
        const float BTN_W = 34.f;
        const float BTN_SPACE_W = 68.f;
        const float ROW_GAP = 16.f;
        const float ICON_W = 24.f;
        const float OR_W = 22.f;
        const float COL_GAP = 8.f;
        const float headerH = 42.f;
        const float panelH = headerH + BTN_H * 3.f + ROW_GAP * 2.f + 40.f;

        RoundedRectShape panel({panelW, panelH}, 8.f);
        panel.setPosition({panelX, panelY});
        panel.setFillColor({10, 10, 40, 160});
        panel.setOutlineColor({60, 100, 200, 120});
        panel.setOutlineThickness(1.f);
        m_window.draw(panel);

        sf::Text ctrlTitle(m_font, "CONTROLS", 18u);
        ctrlTitle.setFillColor({150, 200, 255, 200});
        auto ctb = ctrlTitle.getLocalBounds();
        ctrlTitle.setOrigin({ctb.size.x / 2.f, 0.f});
        ctrlTitle.setPosition({cx, panelY + 10.f});
        m_window.draw(ctrlTitle);

        sf::RectangleShape sep({panelW - 20.f, 1.f});
        sep.setPosition({panelX + 10.f, panelY + headerH});
        sep.setFillColor({60, 100, 200, 100});
        m_window.draw(sep);

        auto drawKey = [&](const std::string &lbl, float kx, float ky,
                           float kw = -1.f)
        {
            float w = (kw < 0.f) ? BTN_W : kw;

            RoundedRectShape key({w, BTN_H}, 5.f);
            key.setPosition({kx, ky});
            key.setFillColor({30, 50, 100, 200});
            key.setOutlineColor({100, 160, 255, 220});
            key.setOutlineThickness(1.5f);
            m_window.draw(key);

            sf::RectangleShape shadow({w, 3.f});
            shadow.setPosition({kx, ky + BTN_H + 1.f});
            shadow.setFillColor({0, 0, 0, 70});
            m_window.draw(shadow);

            unsigned fontSize = 14u;
            if (lbl == "<" || lbl == ">")
                fontSize = 26u;

            sf::Text kt(m_font, lbl, fontSize);
            kt.setFillColor({200, 230, 255, 255});
            auto b = kt.getLocalBounds();
            kt.setOrigin({b.position.x + b.size.x * 0.5f,
                          b.position.y + b.size.y * 0.5f});
            kt.setPosition({kx + w * 0.5f, ky + BTN_H * 0.5f});
            m_window.draw(kt);
        };

        auto drawOr = [&](float ox, float oy)
        {
            sf::Text orTxt(m_font, "or", 13u);
            orTxt.setFillColor({140, 160, 200, 180});
            auto b = orTxt.getLocalBounds();
            orTxt.setOrigin({b.size.x / 2.f, b.size.y / 2.f});
            orTxt.setPosition({ox + (OR_W / 2.f) + 1.0f, oy + (BTN_H / 2.f) - 1.0f});
            m_window.draw(orTxt);
        };

        auto drawLabel = [&](const std::string &text, float lx, float ly,
                             sf::Color col = {180, 200, 255, 200})
        {
            sf::Text lbl(m_font, text, 13u);
            lbl.setFillColor(col);
            auto b = lbl.getLocalBounds();
            lbl.setOrigin({0.f, b.size.y / 2.f});
            lbl.setPosition({lx, ly + BTN_H / 2.f});
            m_window.draw(lbl);
        };

        const float row0Y = panelY + headerH + 17.f;
        const float row1Y = row0Y + BTN_H + ROW_GAP + 4.f;
        const float row2Y = row1Y + BTN_H + ROW_GAP + 4.f;
        const float lineX = panelX + 20.f;
        float kx0 = lineX + ICON_W + COL_GAP + 10.f;

        {
            sf::Sprite shipIcon(GFX.player);
            auto isz = GFX.player.getSize();
            float sc = ICON_W / float(std::max(isz.x, isz.y));
            shipIcon.setScale({sc, sc});
            shipIcon.setPosition({lineX, row0Y + (BTN_H - isz.y * sc) / 2.f});
            m_window.draw(shipIcon);
        }
        drawKey("A", kx0, row0Y);
        drawKey("D", kx0 + BTN_W + COL_GAP, row0Y);
        drawOr(kx0 + 2 * BTN_W + COL_GAP + 2.f, row0Y - 4.f);
        drawKey("<", kx0 + BTN_W * 2 + COL_GAP * 2 + OR_W, row0Y);
        drawKey(">", kx0 + BTN_W * 3 + COL_GAP * 3 + OR_W, row0Y);
        drawLabel("MOVE", kx0 + BTN_W * 4 + COL_GAP * 4 + OR_W, row0Y - 2.f);

        {
            sf::Sprite bulletIcon(GFX.bulletPlayer);
            auto bsz = GFX.bulletPlayer.getSize();
            const float BULLET_ICON_SIZE = 12.f;
            float sc = BULLET_ICON_SIZE / float(std::max(bsz.x, bsz.y));
            bulletIcon.setScale({sc, sc});
            bulletIcon.setOrigin({bsz.x / 2.f, bsz.y / 2.f});
            bulletIcon.setPosition({lineX + ICON_W / 2.f, row1Y + BTN_H / 2.f});
            m_window.draw(bulletIcon);
        }
        drawKey("SPACE", kx0, row1Y, BTN_SPACE_W);
        drawOr(kx0 + BTN_SPACE_W + 2.f, row1Y - 2.f);
        drawKey("Z", kx0 + BTN_SPACE_W + OR_W + COL_GAP, row1Y);
        drawLabel("SHOOT", kx0 + BTN_SPACE_W + OR_W + BTN_W + COL_GAP * 2, row1Y - 4.f);

        drawKey("ESC", kx0, row2Y, BTN_W * 1.5f);
        drawLabel("PAUSE", kx0 + BTN_W * 1.5f + COL_GAP, row2Y - 2.f);
    }

    {
        sf::Sprite star(GFX.star);
        star.setScale({0.2f, 0.2f});

        sf::Text hi(m_font, "BEST: " + std::to_string(m_hiScore), 15u);
        hi.setFillColor({255, 200, 60});
        auto tb = hi.getLocalBounds();

        float totalWidth = 10.f + 2.f + tb.size.x;
        float startX = cx - totalWidth / 2.f;

        star.setPosition({startX - 2.f, float(Cfg::H) - 55.f});
        hi.setPosition({startX + 16.f, float(Cfg::H) - 60.f});

        m_window.draw(star);
        m_window.draw(hi);

        float alpha = std::sin(m_menuPulse * 3.f) * 0.5f + 0.5f;
        alpha = 160.f + alpha * 95.f;
        sf::Text hint(m_font, "PRESS ENTER OR SPACE TO START", 14u);
        hint.setFillColor({180, 180, 180, static_cast<std::uint8_t>(alpha)});
        auto pb = hint.getLocalBounds();
        hint.setOrigin({pb.size.x / 2.f, pb.size.y / 2.f});
        hint.setPosition({cx, float(Cfg::H) - 26.f});
        m_window.draw(hint);
    }

    {
        const float sliderCX = float(Cfg::W) - 115.f;
        const float sliderCY = float(Cfg::H) - 38.f;

        sf::Text vlbl(m_font, "VOLUME", 10u);
        vlbl.setFillColor({120, 150, 200, 160});
        auto vb = vlbl.getLocalBounds();
        vlbl.setOrigin({vb.size.x / 2.f, 0.f});
        vlbl.setPosition({sliderCX, sliderCY - 22.f});
        m_window.draw(vlbl);

        drawVolumeSlider(sliderCX, sliderCY, 110.f);
    }
}

void Game::drawPause()
{
    sf::RectangleShape overlay({float(Cfg::W), float(Cfg::H)});
    overlay.setFillColor({0, 0, 20, 160});
    m_window.draw(overlay);

    const float cx = float(Cfg::W) / 2.f;
    const float cy = float(Cfg::H) / 2.f;

    sf::Text title(m_font, "PAUSED", 42u);
    title.setFillColor({255, 255, 255, 240});
    title.setOutlineColor({0, 0, 40, 200});
    title.setOutlineThickness(3.f);
    auto tb = title.getLocalBounds();
    title.setOrigin({tb.size.x / 2.f, tb.size.y / 2.f});
    title.setPosition({cx, cy - 80.f});
    m_window.draw(title);

    sf::RectangleShape sep({220.f, 1.f});
    sep.setFillColor({80, 120, 255, 80});
    sep.setPosition({cx - 110.f, cy - 28.f});
    m_window.draw(sep);

    drawActionRow(cx, cy - 12.f, {"ESC", "P"}, "Resume");
    drawActionRow(cx, cy + 42.f, {"M"}, "Menu");

    {
        sf::RectangleShape sep2({220.f, 1.f});
        sep2.setFillColor({60, 80, 180, 60});
        sep2.setPosition({cx - 110.f, cy + 95.f});
        m_window.draw(sep2);

        sf::Text vlbl(m_font, "VOLUME", 12u);
        vlbl.setFillColor({140, 170, 220, 180});
        auto vb = vlbl.getLocalBounds();
        vlbl.setOrigin({vb.size.x / 2.f, 0.f});
        vlbl.setPosition({cx, cy + 102.f});
        m_window.draw(vlbl);

        drawVolumeSlider(cx, cy + 128.f, 160.f);
    }
}

void Game::drawGameOver()
{
    sf::RectangleShape overlay({float(Cfg::W), float(Cfg::H)});
    overlay.setFillColor({30, 0, 0, 180});
    m_window.draw(overlay);

    const float cx = float(Cfg::W) / 2.f;
    const float midY = float(Cfg::H) / 2.f;
    float pulse = std::sin(m_menuPulse * 3.f) * 0.5f + 0.5f;

    sf::Text t(m_font, "GAME OVER", 46u);
    t.setFillColor({255, static_cast<std::uint8_t>(30 + pulse * 40), 30, 255});
    t.setOutlineColor({100, 0, 0, 255});
    t.setOutlineThickness(3.f);
    auto b = t.getLocalBounds();
    t.setOrigin({b.size.x / 2.f, b.size.y / 2.f});
    t.setPosition({cx, midY - 110.f});
    m_window.draw(t);

    std::ostringstream ss;
    ss << "SCORE: " << std::setw(8) << std::setfill('0') << m_player.getScore();
    sf::Text sc(m_font, ss.str(), 20u);
    sc.setFillColor({200, 200, 255, 220});
    auto sb = sc.getLocalBounds();
    sc.setOrigin({sb.size.x / 2.f, sb.size.y / 2.f});
    sc.setPosition({cx, midY - 30.f});
    m_window.draw(sc);

    if (m_player.getScore() >= m_hiScore && m_player.getScore() > 0)
    {
        sf::Text hi(m_font, "NEW HI-SCORE!", 18u);
        hi.setFillColor({255, 220, 50, static_cast<std::uint8_t>(180 + pulse * 75)});
        auto hb = hi.getLocalBounds();
        hi.setOrigin({hb.size.x / 2.f, hb.size.y / 2.f});
        hi.setPosition({cx, midY + 12.f});
        m_window.draw(hi);
    }

    const float actionsY = midY + 65.f;
    drawActionRow(cx, actionsY, {"R"}, "Retry");
    drawActionRow(cx, actionsY + 50.f, {"ENTER", "SPACE"}, "Menu");
}

void Game::drawWin()
{
    float pulse = std::sin(m_menuPulse * 2.5f) * 0.5f + 0.5f;

    sf::RectangleShape overlay({float(Cfg::W), float(Cfg::H)});
    overlay.setFillColor({0, 20, 40, 160});
    m_window.draw(overlay);

    const float cx = float(Cfg::W) / 2.f;
    const float midY = float(Cfg::H) / 2.f;

    sf::Text t(m_font, "YOU WIN!", 48u);
    std::uint8_t gr = static_cast<std::uint8_t>(80 + pulse * 175);
    t.setFillColor({gr, 255, static_cast<std::uint8_t>(gr / 2), 255});
    t.setOutlineColor({0, 80, 40, 255});
    t.setOutlineThickness(3.f);
    auto b = t.getLocalBounds();
    t.setOrigin({b.size.x / 2.f, b.size.y / 2.f});
    t.setPosition({cx, midY - 105.f});
    m_window.draw(t);

    sf::Text sub(m_font, "ALL WAVES CLEARED!", 20u);
    sub.setFillColor({200, 255, 200, 220});
    auto subB = sub.getLocalBounds();
    sub.setOrigin({subB.size.x / 2.f, subB.size.y / 2.f});
    sub.setPosition({cx, midY - 28.f});
    m_window.draw(sub);

    std::ostringstream ss;
    ss << "FINAL SCORE: " << m_player.getScore();
    sf::Text sc(m_font, ss.str(), 18u);
    sc.setFillColor({255, 220, 80, 220});
    auto scb = sc.getLocalBounds();
    sc.setOrigin({scb.size.x / 2.f, scb.size.y / 2.f});
    sc.setPosition({cx, midY + 16.f});
    m_window.draw(sc);

    drawActionRow(cx, midY + 72.f, {"ENTER", "SPACE"}, "Menu");
}