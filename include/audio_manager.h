#pragma once

#include <SFML/Audio.hpp>
#include <map>
#include <memory>
#include <string>

class SoundCache {
public:
    sf::SoundBuffer& get(const std::string& path);
    void clear();

private:
    std::map<std::string, sf::SoundBuffer> buffers;
};

class AudioManager {
public:
    AudioManager();

    void playBGM(const std::string& path, bool loop = true, float volume = 70.f);
    void stopBGM();
    void pauseBGM();
    void resumeBGM();
    void setBGMVolume(float volume);
    float getBGMVolume() const;

    void playSFX(const std::string& path, float volume = 100.f);
    void stopAllSFX();

    SoundCache& getSoundCache();

private:
    std::unique_ptr<sf::Music> currentBGM;
    std::vector<std::unique_ptr<sf::Sound>> activeSounds;
    SoundCache soundCache;
    float bgmVolume = 70.f;

    void cleanupInactiveSounds();
};
