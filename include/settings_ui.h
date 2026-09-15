#pragma once

#include "settings.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

enum class SettingType {
    VOLUME_BGM,
    VOLUME_SFX,
    TEXT_SPEED,
    BRIGHTNESS,
    FULLSCREEN,
    RESOLUTION,
    AUTO_ADVANCE,
    RESET,
    BACK
};

struct SettingOption {
    SettingType type;
    std::string label;
    std::string value;
    bool isSlider = false;  // true for volume/brightness, false for toggle
};

class SettingsUI {
public:
    SettingsUI(float screenWidth, float screenHeight, SettingsManager& settingsManager);

    void render(sf::RenderWindow& window, sf::Font& font);
    void update(float deltaTime);

    // Input handling
    void handleUp();
    void handleDown();
    void handleLeft();
    void handleRight();
    void handleConfirm();
    void handleBack();

    // State
    bool isOpen() const { return isMenuOpen; }
    void open();
    void close();

    SettingsManager& getSettingsManager() { return settingsManager; }

private:
    float screenWidth;
    float screenHeight;
    bool isMenuOpen = false;
    int selectedIndex = 0;

    std::vector<SettingOption> options;
    SettingsManager& settingsManager;

    void buildOptions();
    void updateOption(SettingType type, bool increase);
    void drawOption(sf::RenderWindow& window, sf::Font& font,
                   const SettingOption& option, bool selected, float yPos);
    std::string formatValue(SettingType type) const;
};
