#include "../include/settings.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <cmath>

namespace fs = std::filesystem;

std::string GameSettings::toString() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"bgmVolume\": " << bgmVolume << ",\n";
    oss << "  \"sfxVolume\": " << sfxVolume << ",\n";
    oss << "  \"fullscreen\": " << (fullscreen ? "true" : "false") << ",\n";
    oss << "  \"resolutionWidth\": " << resolutionWidth << ",\n";
    oss << "  \"resolutionHeight\": " << resolutionHeight << ",\n";
    oss << "  \"textSpeed\": " << textSpeed << ",\n";
    oss << "  \"autoAdvance\": " << (autoAdvance ? "true" : "false") << ",\n";
    oss << "  \"autoAdvanceDelay\": " << autoAdvanceDelay << ",\n";
    oss << "  \"brightness\": " << brightness << ",\n";
    oss << "  \"screenShake\": " << (screenShake ? "true" : "false") << "\n";
    oss << "}\n";
    return oss.str();
}

GameSettings GameSettings::fromString(const std::string& json) {
    GameSettings settings;

    // Simple JSON parsing (not production-grade, but works for our needs)
    auto extractFloat = [&json](const std::string& key) -> float {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return 0.0f;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return 0.0f;
        pos = json.find_first_not_of(" \t\n", pos + 1);
        size_t endPos = json.find_first_of(",}\n", pos);
        try {
            return std::stof(json.substr(pos, endPos - pos));
        } catch (...) {
            return 0.0f;
        }
    };

    auto extractBool = [&json](const std::string& key) -> bool {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return false;
        pos = json.find(':', pos);
        if (pos == std::string::npos) return false;
        std::string remainder = json.substr(pos);
        return remainder.find("true") < remainder.find("false");
    };

    settings.bgmVolume = extractFloat("bgmVolume");
    settings.sfxVolume = extractFloat("sfxVolume");
    settings.fullscreen = extractBool("fullscreen");
    settings.resolutionWidth = static_cast<int>(extractFloat("resolutionWidth"));
    settings.resolutionHeight = static_cast<int>(extractFloat("resolutionHeight"));
    settings.textSpeed = extractFloat("textSpeed");
    settings.autoAdvance = extractBool("autoAdvance");
    settings.autoAdvanceDelay = extractFloat("autoAdvanceDelay");
    settings.brightness = extractFloat("brightness");
    settings.screenShake = extractBool("screenShake");

    return settings;
}

SettingsManager::SettingsManager(const std::string& path)
    : configPath(path) {
}

bool SettingsManager::loadSettings() {
    if (!fs::exists(configPath)) {
        std::cout << "⚠ Settings file not found: " << configPath << "\n";
        return false;
    }

    try {
        std::ifstream file(configPath);
        if (!file) return false;

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string json = buffer.str();
        file.close();

        settings = GameSettings::fromString(json);
        std::cout << "✓ Settings loaded from " << configPath << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading settings: " << e.what() << "\n";
        return false;
    }
}

bool SettingsManager::saveSettings() {
    try {
        // Create directory if it doesn't exist
        fs::path dir = fs::path(configPath).parent_path();
        if (!dir.empty() && !fs::exists(dir)) {
            fs::create_directories(dir);
        }

        std::ofstream file(configPath);
        if (!file) return false;

        file << settings.toString();
        file.close();

        std::cout << "✓ Settings saved to " << configPath << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving settings: " << e.what() << "\n";
        return false;
    }
}

void SettingsManager::resetToDefaults() {
    settings = GameSettings();
    std::cout << "✓ Settings reset to defaults\n";
}

void SettingsManager::applySettings() {
    // Clamp values to valid ranges
    settings.bgmVolume = std::clamp(settings.bgmVolume, 0.0f, 100.0f);
    settings.sfxVolume = std::clamp(settings.sfxVolume, 0.0f, 100.0f);
    settings.textSpeed = std::clamp(settings.textSpeed, 0.5f, 3.0f);
    settings.brightness = std::clamp(settings.brightness, 0.5f, 1.5f);
    settings.autoAdvanceDelay = std::clamp(settings.autoAdvanceDelay, 1.0f, 10.0f);

    std::cout << "✓ Settings applied\n";
}
