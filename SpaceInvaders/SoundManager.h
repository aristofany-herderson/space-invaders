#pragma once
#include <SFML/Audio.hpp>
#include <array>
#include <memory>
#include <string>
#include <vector>

enum class SoundId
{
    Shoot,
    EnemyShoot,
    Explosion,
    ExplosionBig,
    Hit,
    ShieldHit,
    PlayerHit,
    Powerup,
    UfoLoop,
    WaveStart,
    GameOver,
    Win,

    Count
};

class SoundManager
{
public:
    SoundManager();

    static SoundManager &instance()
    {
        static SoundManager s;
        return s;
    }

    SoundManager(const SoundManager &) = delete;
    SoundManager &operator=(const SoundManager &) = delete;

    void loadAll();

    void play(SoundId id, float vol = 80.f, float pitch = 1.f);

    void startLoop(SoundId id, float vol = 55.f);

    void stopLoop();

    void startMusic(const std::string &file, float vol = 35.f);
    void stopMusic();

    void setMasterVolume(float v);
    float getMasterVolume() const { return m_masterVol; }

private:
    static constexpr std::size_t kSoundCount = static_cast<std::size_t>(SoundId::Count);
    static constexpr std::size_t kVoicePoolSize = 32;

    static const char *fileNameFor(SoundId id);

    std::array<sf::SoundBuffer, kSoundCount> m_buffers;
    std::array<bool, kSoundCount> m_loaded{};

    std::vector<sf::Sound> m_pool;
    std::vector<float> m_poolBaseVol;
    std::size_t m_nextVoice = 0;

    float m_loopBaseVol = 55.f;
    float m_musicBaseVol = 35.f;
    float m_masterVol = 80.f;

    std::unique_ptr<sf::Sound> m_loopSound;
    SoundId m_loopId = SoundId::Count;

    std::unique_ptr<sf::Music> m_music;
};

#define SFX SoundManager::instance()
