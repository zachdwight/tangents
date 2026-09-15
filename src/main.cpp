// Tangents - Visual Novel Engine
// Modern C++ with SFML, modular architecture

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include "../include/script_parser.h"
#include "../include/story_engine.h"
#include "../include/ui_renderer.h"
#include "../include/audio_manager.h"
#include "../include/save_manager.h"
#include "../include/relationship_ui.h"

namespace fs = std::filesystem;

enum class GameUIState {
    NORMAL,
    SAVE_MENU,
    LOAD_MENU,
    BACKLOG
};

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
        SaveManager saveManager("saves");
        RelationshipUI relationshipUI(1920.f, 1080.f);

        // Pre-load font to avoid loading from disk every frame
        sf::Font& uiFont = renderer.getFontCache().get("assets/fonts/Roboto_Condensed-Regular.ttf");

        // Set up character relationships (example characters)
        relationshipUI.setCharacters({"Alice", "Bob", "Charlie"});

        // Initialize example relationships
        engine.getMutableGameState().setRelationship("Alice", 60.0);
        engine.getMutableGameState().setRelationship("Bob", 40.0);
        engine.getMutableGameState().setRelationship("Charlie", 75.0);

        std::cout << "Starting game...\n";
        sf::Clock clock;
        GameUIState uiState = GameUIState::NORMAL;
        int selectedSaveSlot = 1;

        while (window.isOpen()) {
            while (const auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                } else if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyEvent->code == sf::Keyboard::Key::Escape) {
                        if (uiState != GameUIState::NORMAL) {
                            uiState = GameUIState::NORMAL;
                        } else {
                            window.close();
                        }
                    } else if (uiState == GameUIState::NORMAL) {
                        if (keyEvent->code == sf::Keyboard::Key::H) {
                            uiState = GameUIState::BACKLOG;
                        } else if (keyEvent->code == sf::Keyboard::Key::R) {
                            // Toggle relationship display would go here
                            // For now, always show relationships
                        } else if (keyEvent->code == sf::Keyboard::Key::S) {
                            uiState = GameUIState::SAVE_MENU;
                            selectedSaveSlot = 1;
                        } else if (keyEvent->code == sf::Keyboard::Key::L) {
                            uiState = GameUIState::LOAD_MENU;
                            selectedSaveSlot = 1;
                        } else if (keyEvent->code == sf::Keyboard::Key::Space) {
                            const auto& node = engine.getCurrentNode();
                            if (node.choices.empty() && !node.next.empty()) {
                                engine.advanceNode();
                                saveManager.autosave(engine);
                            }
                        }
                    } else if (uiState == GameUIState::BACKLOG) {
                        if (keyEvent->code == sf::Keyboard::Key::H) {
                            uiState = GameUIState::NORMAL;
                        }
                    } else if (uiState == GameUIState::SAVE_MENU) {
                        if (keyEvent->code == sf::Keyboard::Key::Up) {
                            selectedSaveSlot = std::max(1, selectedSaveSlot - 1);
                        } else if (keyEvent->code == sf::Keyboard::Key::Down) {
                            selectedSaveSlot = std::min(10, selectedSaveSlot + 1);
                        } else if (keyEvent->code == sf::Keyboard::Key::Enter) {
                            try {
                                const auto& node = engine.getCurrentNode();
                                saveManager.saveGame(engine, selectedSaveSlot, node.speaker);
                                uiState = GameUIState::NORMAL;
                            } catch (const std::exception& e) {
                                std::cerr << "Save failed: " << e.what() << "\n";
                            }
                        }
                    } else if (uiState == GameUIState::LOAD_MENU) {
                        if (keyEvent->code == sf::Keyboard::Key::Up) {
                            selectedSaveSlot = std::max(1, selectedSaveSlot - 1);
                        } else if (keyEvent->code == sf::Keyboard::Key::Down) {
                            selectedSaveSlot = std::min(10, selectedSaveSlot + 1);
                        } else if (keyEvent->code == sf::Keyboard::Key::Enter) {
                            if (saveManager.loadGame(engine, selectedSaveSlot)) {
                                uiState = GameUIState::NORMAL;
                            }
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

            if (uiState == GameUIState::BACKLOG) {
                // Render backlog view
                sf::Text titleText(uiFont, "DIALOGUE HISTORY (Press H to close)", 28);
                titleText.setFillColor(sf::Color::Cyan);
                titleText.setPosition({50.f, 30.f});
                window.draw(titleText);

                const auto& backlog = engine.getBacklog();
                float yPos = 100.f;
                int displayCount = std::min(static_cast<int>(backlog.size()), 20);

                // Display backlog from most recent backwards
                for (int i = displayCount - 1; i >= 0; --i) {
                    const auto& entry = backlog[backlog.size() - 1 - i];

                    sf::Text speakerText(uiFont, entry.speaker + ":", 20);
                    speakerText.setFillColor(sf::Color::Yellow);
                    speakerText.setPosition({50.f, yPos});
                    window.draw(speakerText);

                    sf::Text contentText(uiFont, entry.text, 18);
                    contentText.setFillColor(sf::Color::White);
                    contentText.setPosition({250.f, yPos});
                    renderer.setWrappedText(contentText, entry.text, 1600.f);
                    window.draw(contentText);

                    yPos += 50.f;
                    if (yPos > 1000.f) break;  // Stop if below screen
                }
            } else if (uiState == GameUIState::NORMAL) {
                // Render normal dialogue view
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

                sf::Text speakerText(uiFont, currentNode.speaker, 32);
                speakerText.setFillColor(sf::Color::White);
                speakerText.setPosition({50.f, 50.f});
                window.draw(speakerText);

                std::string dialogueStr = currentNode.dialogue.empty() ?
                    (currentNode.descriptionPages.empty() ? "..." : currentNode.descriptionPages[0]) :
                    currentNode.dialogue;
                sf::Text dialogueText(uiFont, dialogueStr, 24);
                dialogueText.setFillColor(sf::Color::White);
                dialogueText.setPosition({50.f, 100.f});
                renderer.setWrappedText(dialogueText, dialogueStr, 1800.f);
                window.draw(dialogueText);

                if (!currentNode.choices.empty()) {
                    float choiceY = 850.f;
                    for (const auto& choice : currentNode.choices) {
                        std::string choiceLabel = "[" + choice.key + "] " + choice.text;
                        sf::Text choiceText(uiFont, choiceLabel, 20);
                        choiceText.setFillColor(sf::Color::Yellow);
                        choiceText.setPosition({100.f, choiceY});
                        window.draw(choiceText);
                        choiceY += 40.f;
                    }
                }

                // Show hints
                sf::Text hintsText(uiFont, "[H] History  [S] Save  [L] Load", 16);
                hintsText.setFillColor(sf::Color{200, 200, 200});
                hintsText.setPosition({1400.f, 1040.f});
                window.draw(hintsText);

                // Render relationship meters
                relationshipUI.render(window, uiFont, engine.getGameState());
            } else if (uiState == GameUIState::SAVE_MENU) {
                sf::Text titleText(uiFont, "SAVE GAME", 40);
                titleText.setFillColor(sf::Color::Cyan);
                titleText.setPosition({800.f, 50.f});
                window.draw(titleText);

                auto saves = saveManager.listSaves();
                float yPos = 150.f;

                for (const auto& save : saves) {
                    sf::Text slotText(uiFont, std::string(save.slot == selectedSaveSlot ? "> " : "  ") +
                                      "Slot " + std::to_string(save.slot), 28);
                    slotText.setFillColor(save.slot == selectedSaveSlot ? sf::Color::Yellow : sf::Color::White);
                    slotText.setPosition({200.f, yPos});
                    window.draw(slotText);

                    if (save.exists) {
                        sf::Text infoText(uiFont, save.title + " - " + save.timestamp, 20);
                        infoText.setFillColor(sf::Color{200, 200, 200});
                        infoText.setPosition({600.f, yPos});
                        window.draw(infoText);

                        std::ostringstream timeStr;
                        int mins = static_cast<int>(save.playtimeSecs) / 60;
                        int secs = static_cast<int>(save.playtimeSecs) % 60;
                        timeStr << mins << "m " << secs << "s";
                        sf::Text timeText(uiFont, timeStr.str(), 18);
                        timeText.setFillColor(sf::Color{150, 150, 150});
                        timeText.setPosition({1600.f, yPos});
                        window.draw(timeText);
                    }

                    yPos += 70.f;
                }

                sf::Text instructionText(uiFont, "↑↓ Navigate  [Enter] Save  [Esc] Cancel", 18);
                instructionText.setFillColor(sf::Color::Green);
                instructionText.setPosition({200.f, 950.f});
                window.draw(instructionText);
            } else if (uiState == GameUIState::LOAD_MENU) {
                sf::Text titleText(uiFont, "LOAD GAME", 40);
                titleText.setFillColor(sf::Color::Cyan);
                titleText.setPosition({800.f, 50.f});
                window.draw(titleText);

                auto saves = saveManager.listSaves();
                float yPos = 150.f;

                for (const auto& save : saves) {
                    sf::Text slotText(uiFont, std::string(save.slot == selectedSaveSlot ? "> " : "  ") +
                                      "Slot " + std::to_string(save.slot), 28);
                    slotText.setFillColor(save.slot == selectedSaveSlot ? sf::Color::Yellow : sf::Color::White);
                    slotText.setPosition({200.f, yPos});
                    window.draw(slotText);

                    if (save.exists) {
                        sf::Text infoText(uiFont, save.title + " - " + save.timestamp, 20);
                        infoText.setFillColor(sf::Color{200, 200, 200});
                        infoText.setPosition({600.f, yPos});
                        window.draw(infoText);

                        std::ostringstream timeStr;
                        int mins = static_cast<int>(save.playtimeSecs) / 60;
                        int secs = static_cast<int>(save.playtimeSecs) % 60;
                        timeStr << mins << "m " << secs << "s";
                        sf::Text timeText(uiFont, timeStr.str(), 18);
                        timeText.setFillColor(sf::Color{150, 150, 150});
                        timeText.setPosition({1600.f, yPos});
                        window.draw(timeText);
                    } else {
                        sf::Text emptyText(uiFont, "(Empty)", 20);
                        emptyText.setFillColor(sf::Color{100, 100, 100});
                        emptyText.setPosition({600.f, yPos});
                        window.draw(emptyText);
                    }

                    yPos += 70.f;
                }

                sf::Text instructionText(uiFont, "↑↓ Navigate  [Enter] Load  [Esc] Cancel", 18);
                instructionText.setFillColor(sf::Color::Green);
                instructionText.setPosition({200.f, 950.f});
                window.draw(instructionText);
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
