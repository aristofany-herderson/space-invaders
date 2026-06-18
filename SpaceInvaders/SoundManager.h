#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

class SoundManager
{
public:
    SoundManager();
    static SoundManager &instance()
    {
        static SoundManager s;
        return s;
    }

    void loadAll();
    void play(const std::string &name,
              float vol = 80.f,
              float pitch = 1.f);
    void startLoop(const std::string &name, float vol = 55.f);
    void stopLoop();

    void startMusic(const std::string &file, float vol = 35.f);
    void stopMusic();

    void setMasterVolume(float v);
    float getMasterVolume() const { return m_masterVol; }

private:
    std::unordered_map<std::string, sf::SoundBuffer> m_buffers;

    std::vector<sf::Sound> m_pool;
    size_t m_nextVoice = 0;

    std::vector<float> m_poolBaseVol;
    float m_loopBaseVol = 55.f;
    float m_musicBaseVol = 35.f;

    float m_masterVol = 80.f;

    std::unique_ptr<sf::Sound> m_loopSound;
    std::string m_loopName;

    std::unique_ptr<sf::Music> m_music;
};

#define SFX SoundManager::instance()