#include "../include/audio_manager.h"
#include <stdexcept>

sf::SoundBuffer& SoundCache::get(const std::string& path) {
    auto it = buffers.find(path);
    if (it != buffers.end()) return it->second;

    sf::SoundBuffer buf;
    if (!buf.loadFromFile(path)) {
        throw std::runtime_error("Failed to load sound: " + path);
    }
    auto ins = buffers.emplace(path, std::move(buf));
    return ins.first->second;
}

void SoundCache::clear() {
    buffers.clear();
}

AudioManager::AudioManager()
    : currentBGM(std::make_unique<sf::Music>()), bgmVolume(70.f) {}

void AudioManager::playBGM(const std::string& path, bool loop, float volume) {
    if (path.empty()) {
        stopBGM();
        return;
    }

    if (!currentBGM->openFromFile(path)) {
        throw std::runtime_error("Failed to load BGM: " + path);
    }

    currentBGM->setLooping(loop);
    setBGMVolume(volume);
    currentBGM->play();
}

void AudioManager::stopBGM() {
    if (currentBGM) {
        currentBGM->stop();
    }
}

void AudioManager::pauseBGM() {
    if (currentBGM) {
        currentBGM->pause();
    }
}

void AudioManager::resumeBGM() {
    if (currentBGM && currentBGM->getStatus() == sf::Music::Status::Paused) {
        currentBGM->play();
    }
}

void AudioManager::setBGMVolume(float volume) {
    bgmVolume = std::clamp(volume, 0.f, 100.f);
    if (currentBGM) {
        currentBGM->setVolume(bgmVolume);
    }
}

float AudioManager::getBGMVolume() const {
    return bgmVolume;
}

void AudioManager::playSFX(const std::string& path, float volume) {
    if (path.empty()) return;

    try {
        auto& buffer = soundCache.get(path);
        auto sound = std::make_unique<sf::Sound>(buffer);
        sound->setVolume(std::clamp(volume, 0.f, 100.f));
        sound->play();
        activeSounds.push_back(std::move(sound));
        cleanupInactiveSounds();
    } catch (const std::exception&) {
        // Silently fail for missing SFX files
    }
}

void AudioManager::stopAllSFX() {
    activeSounds.clear();
}

SoundCache& AudioManager::getSoundCache() {
    return soundCache;
}

void AudioManager::cleanupInactiveSounds() {
    activeSounds.erase(
        std::remove_if(
            activeSounds.begin(),
            activeSounds.end(),
            [](const std::unique_ptr<sf::Sound>& s) { return s->getStatus() == sf::Sound::Status::Stopped; }),
        activeSounds.end());
}
