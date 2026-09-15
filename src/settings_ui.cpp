#include "../include/settings_ui.h"
#include <sstream>
#include <iomanip>

SettingsUI::SettingsUI(float width, float height, SettingsManager& mgr)
    : screenWidth(width), screenHeight(height), settingsManager(mgr) {
    buildOptions();
}

void SettingsUI::buildOptions() {
    options.clear();
    options.push_back({SettingType::VOLUME_BGM, "BGM Volume", "", true});
    options.push_back({SettingType::VOLUME_SFX, "SFX Volume", "", true});
    options.push_back({SettingType::TEXT_SPEED, "Text Speed", "", false});
    options.push_back({SettingType::BRIGHTNESS, "Brightness", "", true});
    options.push_back({SettingType::FULLSCREEN, "Fullscreen", "", false});
    options.push_back({SettingType::AUTO_ADVANCE, "Auto Advance", "", false});
    options.push_back({SettingType::RESET, "Reset to Defaults", "", false});
    options.push_back({SettingType::BACK, "Back to Game", "", false});
}

std::string SettingsUI::formatValue(SettingType type) const {
    const GameSettings& s = settingsManager.getSettings();

    switch (type) {
        case SettingType::VOLUME_BGM: {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(0) << s.bgmVolume << "%";
            return oss.str();
        }
        case SettingType::VOLUME_SFX: {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(0) << s.sfxVolume << "%";
            return oss.str();
        }
        case SettingType::TEXT_SPEED: {
            if (s.textSpeed < 0.75f) return "Slow";
            if (s.textSpeed < 1.25f) return "Normal";
            return "Fast";
        }
        case SettingType::BRIGHTNESS: {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << s.brightness;
            return oss.str();
        }
        case SettingType::FULLSCREEN:
            return s.fullscreen ? "Yes" : "No";
        case SettingType::AUTO_ADVANCE:
            return s.autoAdvance ? "On" : "Off";
        case SettingType::RESOLUTION:
            return std::to_string(s.resolutionWidth) + "x" + std::to_string(s.resolutionHeight);
        default:
            return "";
    }
}

void SettingsUI::updateOption(SettingType type, bool increase) {
    GameSettings& s = settingsManager.getSettings();
    float delta = increase ? 5.0f : -5.0f;

    switch (type) {
        case SettingType::VOLUME_BGM:
            s.bgmVolume = std::clamp(s.bgmVolume + delta, 0.0f, 100.0f);
            break;
        case SettingType::VOLUME_SFX:
            s.sfxVolume = std::clamp(s.sfxVolume + delta, 0.0f, 100.0f);
            break;
        case SettingType::TEXT_SPEED:
            s.textSpeed = increase ?
                (s.textSpeed < 0.75f ? 1.0f : 2.0f) :
                (s.textSpeed > 1.25f ? 1.0f : 0.5f);
            break;
        case SettingType::BRIGHTNESS:
            s.brightness = std::clamp(s.brightness + delta * 0.01f, 0.5f, 1.5f);
            break;
        case SettingType::FULLSCREEN:
            s.fullscreen = !s.fullscreen;
            break;
        case SettingType::AUTO_ADVANCE:
            s.autoAdvance = !s.autoAdvance;
            break;
        default:
            break;
    }
}

void SettingsUI::handleUp() {
    if (selectedIndex > 0) selectedIndex--;
}

void SettingsUI::handleDown() {
    if (selectedIndex < static_cast<int>(options.size()) - 1) selectedIndex++;
}

void SettingsUI::handleLeft() {
    if (selectedIndex < static_cast<int>(options.size()) - 2) {
        updateOption(options[selectedIndex].type, false);
    }
}

void SettingsUI::handleRight() {
    if (selectedIndex < static_cast<int>(options.size()) - 2) {
        updateOption(options[selectedIndex].type, true);
    }
}

void SettingsUI::handleConfirm() {
    SettingType type = options[selectedIndex].type;

    if (type == SettingType::RESET) {
        settingsManager.resetToDefaults();
        buildOptions();
    } else if (type == SettingType::BACK) {
        close();
        settingsManager.saveSettings();
    }
}

void SettingsUI::handleBack() {
    close();
    settingsManager.saveSettings();
}

void SettingsUI::open() {
    isMenuOpen = true;
    selectedIndex = 0;
}

void SettingsUI::close() {
    isMenuOpen = false;
}

void SettingsUI::update(float /*deltaTime*/) {
    // Update values display
    for (auto& option : options) {
        if (option.type != SettingType::RESET && option.type != SettingType::BACK) {
            option.value = formatValue(option.type);
        }
    }
}

void SettingsUI::drawOption(sf::RenderWindow& window, sf::Font& font,
                           const SettingOption& option, bool selected, float yPos) {
    // Label
    sf::Text labelText(font, option.label, 24);
    labelText.setFillColor(selected ? sf::Color::Yellow : sf::Color::White);
    labelText.setPosition({200.f, yPos});
    window.draw(labelText);

    // Value (for sliders/toggles)
    if (!option.value.empty()) {
        sf::Text valueText(font, option.value, 20);
        valueText.setFillColor(selected ? sf::Color::Yellow : sf::Color{200, 200, 200});
        valueText.setPosition({800.f, yPos});
        window.draw(valueText);

        // Arrows for adjustable options
        if (option.isSlider || option.type == SettingType::TEXT_SPEED ||
            option.type == SettingType::FULLSCREEN || option.type == SettingType::AUTO_ADVANCE) {
            sf::Text leftArrow(font, selected ? "< " : "  ", 18);
            leftArrow.setFillColor(selected ? sf::Color::Cyan : sf::Color{100, 100, 100});
            leftArrow.setPosition({750.f, yPos});
            window.draw(leftArrow);

            sf::Text rightArrow(font, selected ? " >" : "  ", 18);
            rightArrow.setFillColor(selected ? sf::Color::Cyan : sf::Color{100, 100, 100});
            rightArrow.setPosition({1050.f, yPos});
            window.draw(rightArrow);
        }
    }
}

void SettingsUI::render(sf::RenderWindow& window, sf::Font& font) {
    if (!isMenuOpen) return;

    // Semi-transparent background
    sf::RectangleShape bg(sf::Vector2f(screenWidth, screenHeight));
    bg.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(bg);

    // Title
    sf::Text titleText(font, "SETTINGS", 48);
    titleText.setFillColor(sf::Color::Cyan);
    titleText.setPosition({screenWidth / 2.f - 150.f, 50.f});
    window.draw(titleText);

    // Options
    float yPos = 150.f;
    for (int i = 0; i < static_cast<int>(options.size()); ++i) {
        drawOption(window, font, options[i], i == selectedIndex, yPos);
        yPos += 80.f;
    }

    // Instructions
    sf::Text instructionsText(font, "↑↓ Navigate  ← → Adjust  [Enter] Confirm  [Esc] Close", 16);
    instructionsText.setFillColor(sf::Color{150, 150, 150});
    instructionsText.setPosition({100.f, screenHeight - 50.f});
    window.draw(instructionsText);
}
