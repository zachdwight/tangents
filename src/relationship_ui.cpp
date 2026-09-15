#include "../include/relationship_ui.h"
#include <sstream>
#include <iomanip>

RelationshipUI::RelationshipUI(float width, float height)
    : screenWidth(width), screenHeight(height) {
}

void RelationshipUI::setCharacters(const std::vector<std::string>& chars) {
    trackedCharacters = chars;
}

sf::Color RelationshipUI::getAffinityColor(double affinity) const {
    // Red (0) -> Yellow (50) -> Green (100)
    if (affinity < 50.0) {
        // Red to Yellow: (255, 0, 0) -> (255, 255, 0)
        float ratio = affinity / 50.0;
        return sf::Color(
            255,
            static_cast<unsigned char>(255.0 * ratio),
            0
        );
    } else {
        // Yellow to Green: (255, 255, 0) -> (0, 255, 0)
        float ratio = (affinity - 50.0) / 50.0;
        return sf::Color(
            static_cast<unsigned char>(255.0 * (1.0 - ratio)),
            255,
            0
        );
    }
}

std::string RelationshipUI::getAffinityLabel(double affinity) const {
    if (affinity < 20.0) return "Hostile";
    if (affinity < 40.0) return "Cold";
    if (affinity < 60.0) return "Neutral";
    if (affinity < 80.0) return "Friendly";
    return "Devoted";
}

void RelationshipUI::drawMeter(sf::RenderWindow& window, sf::Font& font,
                              const std::string& character, double affinity, float yPos) {
    // Character name
    sf::Text nameText(font, character, 16);
    nameText.setFillColor(sf::Color::White);
    nameText.setPosition({screenWidth - METER_WIDTH - 150.f, yPos});
    window.draw(nameText);

    // Background bar (dark gray)
    sf::RectangleShape bgBar(sf::Vector2f(METER_WIDTH, METER_HEIGHT));
    bgBar.setFillColor(sf::Color{50, 50, 50});
    bgBar.setPosition({screenWidth - METER_WIDTH - 10.f, yPos + 20.f});
    window.draw(bgBar);

    // Affinity bar (colored based on relationship)
    float fillWidth = (affinity / 100.0) * METER_WIDTH;
    sf::RectangleShape affinityBar(sf::Vector2f(fillWidth, METER_HEIGHT));
    affinityBar.setFillColor(getAffinityColor(affinity));
    affinityBar.setPosition({screenWidth - METER_WIDTH - 10.f, yPos + 20.f});
    window.draw(affinityBar);

    // Border
    sf::RectangleShape border(sf::Vector2f(METER_WIDTH, METER_HEIGHT));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color{150, 150, 150});
    border.setOutlineThickness(1.f);
    border.setPosition({screenWidth - METER_WIDTH - 10.f, yPos + 20.f});
    window.draw(border);

    // Affinity value and label
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << affinity << "%";
    sf::Text valueText(font, oss.str(), 14);
    valueText.setFillColor(sf::Color::White);
    valueText.setPosition({screenWidth - METER_WIDTH + 150.f, yPos + 20.f});
    window.draw(valueText);

    sf::Text labelText(font, getAffinityLabel(affinity), 12);
    labelText.setFillColor(sf::Color{200, 200, 200});
    labelText.setPosition({screenWidth - METER_WIDTH - 10.f, yPos + 45.f});
    window.draw(labelText);
}

void RelationshipUI::render(sf::RenderWindow& window, sf::Font& font, const GameState& gameState) {
    if (!showRelationships || trackedCharacters.empty()) {
        return;
    }

    // Title
    sf::Text titleText(font, "RELATIONSHIPS", 18);
    titleText.setFillColor(sf::Color::Cyan);
    titleText.setPosition({screenWidth - 400.f, 20.f});
    window.draw(titleText);

    // Draw meters for each character
    float yPos = 50.f;
    for (const auto& character : trackedCharacters) {
        double affinity = gameState.getRelationship(character, 50.0);
        drawMeter(window, font, character, affinity, yPos);
        yPos += 80.f;
    }

    // Hide hint
    sf::Text hideHintText(font, "Press [R] to toggle relationships", 12);
    hideHintText.setFillColor(sf::Color{100, 100, 100});
    hideHintText.setPosition({screenWidth - 400.f, screenHeight - 30.f});
    window.draw(hideHintText);
}
