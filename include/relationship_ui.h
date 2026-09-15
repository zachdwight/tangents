#pragma once

#include "story_engine.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct RelationshipDisplay {
    std::string character;
    double affinity;  // 0-100
};

class RelationshipUI {
public:
    RelationshipUI(float screenWidth, float screenHeight);

    void render(sf::RenderWindow& window, sf::Font& font, const GameState& gameState);
    void setCharacters(const std::vector<std::string>& chars);
    void setShowRelationships(bool show) { showRelationships = show; }

private:
    float screenWidth;
    float screenHeight;
    std::vector<std::string> trackedCharacters;
    bool showRelationships = true;
    static constexpr float METER_WIDTH = 200.f;
    static constexpr float METER_HEIGHT = 20.f;
    static constexpr float PADDING = 10.f;

    sf::Color getAffinityColor(double affinity) const;
    std::string getAffinityLabel(double affinity) const;
    void drawMeter(sf::RenderWindow& window, sf::Font& font,
                   const std::string& character, double affinity, float yPos);
};
