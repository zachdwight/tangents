#pragma once

#include <string>
#include <unordered_map>

struct GameSettings {
    // Audio
    float bgmVolume = 70.0f;      // 0-100
    float sfxVolume = 80.0f;      // 0-100

    // Display
    bool fullscreen = false;
    int resolutionWidth = 1920;
    int resolutionHeight = 1080;

    // Gameplay
    float textSpeed = 1.0f;        // 0.5 (slow), 1.0 (normal), 2.0 (fast)
    bool autoAdvance = false;
    float autoAdvanceDelay = 3.0f; // seconds

    // Visual
    float brightness = 1.0f;       // 0.5-1.5
    bool screenShake = true;

    // Serialization
    std::string toString() const;
    static GameSettings fromString(const std::string& json);
};

class SettingsManager {
public:
    SettingsManager(const std::string& configPath = "config/settings.json");

    // Load/Save
    bool loadSettings();
    bool saveSettings();

    // Access
    GameSettings& getSettings() { return settings; }
    const GameSettings& getSettings() const { return settings; }

    // Helpers
    void resetToDefaults();
    void applySettings();  // Apply to game systems

private:
    GameSettings settings;
    std::string configPath;

    std::string serializeJson() const;
    bool deserializeJson(const std::string& json);
};
