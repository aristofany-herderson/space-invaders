#include "SoundManager.h"
#include <iostream>

SoundManager::SoundManager()
{
    auto dummy = sf::SoundBuffer();

    for (int i = 0; i < 32; ++i)
    {
        m_pool.emplace_back(dummy);
    }
}
void SoundManager::loadAll()
{
    const std::string dir = "assets/sounds/";
    for (const auto &name : {
             "shoot", "enemy_shoot",
             "explosion", "explosion_big",
             "hit", "shield_hit", "player_hit",
             "powerup", "ufo_loop",
             "wave_start", "game_over", "win"})
    {
        sf::SoundBuffer buf;
        std::string path = dir + name + ".wav";
        if (buf.loadFromFile(path))
            m_buffers.emplace(name, std::move(buf));
        else
            std::cerr << "[SoundManager] nao encontrado: " << path << "\n";
    }
}

void SoundManager::play(
    const std::string &name,
    float vol,
    float pitch)
{
    auto it = m_buffers.find(name);

    if (it == m_buffers.end())
        return;

    auto &s = m_pool[m_nextVoice];

    s.stop();

    s.setBuffer(it->second);

    s.setVolume(
        vol * masterVolume / 100.f);

    s.setPitch(pitch);

    s.play();

    m_nextVoice++;

    if (m_nextVoice >= m_pool.size())
        m_nextVoice = 0;
}

void SoundManager::startLoop(const std::string &name, float vol)
{
    if (m_loopName == name && m_loopSound &&
        m_loopSound->getStatus() == sf::Sound::Status::Playing)
        return;

    auto it = m_buffers.find(name);
    if (it == m_buffers.end())
        return;

    m_loopSound = std::make_unique<sf::Sound>(it->second);
    m_loopSound->setVolume(vol * masterVolume / 100.f);
    m_loopSound->setLooping(true);
    m_loopSound->play();
    m_loopName = name;
}

void SoundManager::stopLoop()
{
    if (m_loopSound)
    {
        m_loopSound->stop();
        m_loopSound.reset();
    }
    m_loopName.clear();
}

void SoundManager::startMusic(const std::string &file, float vol)
{
    if (!m_music)
        m_music = std::make_unique<sf::Music>();

    if (!m_music->openFromFile(file))
    {
        std::cerr << "[Music] nao encontrado: "
                  << file << "\n";
        return;
    }

    m_music->setLooping(true);
    m_music->setVolume(vol * masterVolume / 100.f);
    m_music->play();
}

void SoundManager::stopMusic()
{
    if (m_music)
        m_music->stop();
}