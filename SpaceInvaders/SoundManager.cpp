#include "SoundManager.h"
#include <algorithm>
#include <iostream>

namespace
{
    constexpr const char *kSoundDir = "assets/sounds/";
}

const char *SoundManager::fileNameFor(SoundId id)
{
    switch (id)
    {
    case SoundId::Shoot:
        return "shoot";
    case SoundId::EnemyShoot:
        return "enemy_shoot";
    case SoundId::Explosion:
        return "explosion";
    case SoundId::ExplosionBig:
        return "explosion_big";
    case SoundId::Hit:
        return "hit";
    case SoundId::ShieldHit:
        return "shield_hit";
    case SoundId::PlayerHit:
        return "player_hit";
    case SoundId::Powerup:
        return "powerup";
    case SoundId::UfoLoop:
        return "ufo_loop";
    case SoundId::WaveStart:
        return "wave_start";
    case SoundId::GameOver:
        return "game_over";
    case SoundId::Win:
        return "win";
    case SoundId::Count:
        break;
    }
    return "";
}

SoundManager::SoundManager()
{
    sf::SoundBuffer dummy;
    m_pool.reserve(kVoicePoolSize);
    m_poolBaseVol.reserve(kVoicePoolSize);
    for (std::size_t i = 0; i < kVoicePoolSize; ++i)
    {
        m_pool.emplace_back(dummy);
        m_poolBaseVol.push_back(80.f);
    }
}

void SoundManager::loadAll()
{
    for (std::size_t i = 0; i < kSoundCount; ++i)
    {
        const auto id = static_cast<SoundId>(i);
        const std::string path = std::string(kSoundDir) + fileNameFor(id) + ".wav";

        if (m_buffers[i].loadFromFile(path))
        {
            m_loaded[i] = true;
        }
        else
        {
            m_loaded[i] = false;
            std::cerr << "[SoundManager] nao encontrado: " << path << "\n";
        }
    }
}

void SoundManager::play(SoundId id, float vol, float pitch)
{
    const auto idx = static_cast<std::size_t>(id);
    if (idx >= kSoundCount || !m_loaded[idx])
        return;

    auto &s = m_pool[m_nextVoice];
    s.stop();
    s.setBuffer(m_buffers[idx]);
    s.setVolume(vol * m_masterVol / 100.f);
    s.setPitch(pitch);
    s.play();

    m_poolBaseVol[m_nextVoice] = vol;

    if (++m_nextVoice >= m_pool.size())
        m_nextVoice = 0;
}

void SoundManager::startLoop(SoundId id, float vol)
{
    const auto idx = static_cast<std::size_t>(id);
    if (idx >= kSoundCount || !m_loaded[idx])
        return;

    if (m_loopSound && m_loopId == id &&
        m_loopSound->getStatus() == sf::Sound::Status::Playing)
        return;

    m_loopBaseVol = vol;
    m_loopSound = std::make_unique<sf::Sound>(m_buffers[idx]);
    m_loopSound->setVolume(vol * m_masterVol / 100.f);
    m_loopSound->setLooping(true);
    m_loopSound->play();
    m_loopId = id;
}

void SoundManager::stopLoop()
{
    if (m_loopSound)
    {
        m_loopSound->stop();
        m_loopSound.reset();
    }
    m_loopId = SoundId::Count;
}

void SoundManager::startMusic(const std::string &file, float vol)
{
    m_musicBaseVol = vol;

    if (!m_music)
        m_music = std::make_unique<sf::Music>();

    if (!m_music->openFromFile(file))
    {
        std::cerr << "[Music] nao encontrado: " << file << "\n";
        return;
    }

    m_music->setLooping(true);
    m_music->setVolume(vol * m_masterVol / 100.f);
    m_music->play();
}

void SoundManager::stopMusic()
{
    if (m_music)
        m_music->stop();
}

void SoundManager::setMasterVolume(float v)
{
    m_masterVol = std::clamp(v, 0.f, 100.f);

    if (m_music)
        m_music->setVolume(m_musicBaseVol * m_masterVol / 100.f);

    if (m_loopSound)
        m_loopSound->setVolume(m_loopBaseVol * m_masterVol / 100.f);

    for (std::size_t i = 0; i < m_pool.size(); ++i)
        if (m_pool[i].getStatus() == sf::Sound::Status::Playing)
            m_pool[i].setVolume(m_poolBaseVol[i] * m_masterVol / 100.f);
}
