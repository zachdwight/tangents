// Tangents - Visual Novel Engine
// Modern C++ with SFML, modular architecture

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <stdexcept>

#include "../include/script_parser.h"
#include "../include/story_engine.h"
#include "../include/ui_renderer.h"
#include "../include/audio_manager.h"

namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "Loading script from assets/script.toml...\n";
        Script script = ScriptParser::loadFromFile("assets/script.toml");
        std::cout << "Loaded " << script.nodes.size() << " nodes\n";

        StoryEngine engine(script);
        AudioManager audioManager;

        sf::RenderWindow window(sf::VideoMode({1920u, 1080u}), "Tangents - Visual Novel");
        window.setFramerateLimit(60);

        UIRenderer renderer(1920.f, 1080.f);

        std::cout << "Starting game...\n";
        sf::Clock clock;

        while (window.isOpen()) {
            while (const auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                } else if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyEvent->code == sf::Keyboard::Key::Escape) {
                        window.close();
                    } else if (keyEvent->code == sf::Keyboard::Key::Space) {
                        const auto& node = engine.getCurrentNode();
                        if (node.choices.empty() && !node.next.empty()) {
                            engine.advanceNode();
                        }
                    }
                } else if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
                    char c = static_cast<char>(textEvent->unicode);
                    if (c == 'A' || c == 'a') {
                        const auto& node = engine.getCurrentNode();
                        if (!node.choices.empty()) engine.makeChoice("A");
                    } else if (c == 'B' || c == 'b') {
                        const auto& node = engine.getCurrentNode();
                        if (node.choices.size() > 1) engine.makeChoice("B");
                    } else if (c == 'C' || c == 'c') {
                        const auto& node = engine.getCurrentNode();
                        if (node.choices.size() > 2) engine.makeChoice("C");
                    }
                }
            }

            float dt = clock.restart().asSeconds();
            engine.updatePlaytime(dt);

            window.clear(sf::Color::Black);

            const auto& currentNode = engine.getCurrentNode();

            if (!currentNode.bgFile.empty()) {
                try {
                    auto& bgTexture = renderer.getTextureCache().get(currentNode.bgFile);
                    sf::Sprite bgSprite(bgTexture);
                    renderer.scaleSpriteCover(bgSprite, bgTexture, 1920.f, 1080.f);
                    window.draw(bgSprite);
                } catch (...) {
                    // Asset missing, continue
                }
            }

            sf::Font font;
            if (font.openFromFile("assets/fonts/Roboto_Condensed-Regular.ttf")) {
                sf::Text speakerText(font, currentNode.speaker, 32);
                speakerText.setFillColor(sf::Color::White);
                speakerText.setPosition({50.f, 50.f});
                window.draw(speakerText);

                std::string dialogueStr = currentNode.dialogue.empty() ?
                    (currentNode.descriptionPages.empty() ? "..." : currentNode.descriptionPages[0]) :
                    currentNode.dialogue;
                sf::Text dialogueText(font, dialogueStr, 24);
                dialogueText.setFillColor(sf::Color::White);
                dialogueText.setPosition({50.f, 100.f});
                renderer.setWrappedText(dialogueText, dialogueStr, 1800.f);
                window.draw(dialogueText);

                if (!currentNode.choices.empty()) {
                    float choiceY = 850.f;
                    for (const auto& choice : currentNode.choices) {
                        std::string choiceLabel = "[" + choice.key + "] " + choice.text;
                        sf::Text choiceText(font, choiceLabel, 20);
                        choiceText.setFillColor(sf::Color::Yellow);
                        choiceText.setPosition({100.f, choiceY});
                        window.draw(choiceText);
                        choiceY += 40.f;
                    }
                }
            }

            window.display();
        }

        std::cout << "Game ended. Playtime: " << engine.getGameState().playtimeSecs << " seconds\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
