/**
 * Sprite Sheet Integration Example
 *
 * This example demonstrates how to:
 * 1. Load a sprite sheet from a JSON definition
 * 2. Define character animations using sprite sheet frames
 * 3. Render animated characters with expression changes
 *
 * Build: g++ -std=c++20 -I../include sprite_sheet_example.cpp
 *        ../src/sprite_sheet.cpp ../src/animation.cpp
 *        -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system
 */

#include "../include/sprite_sheet.h"
#include "../include/animation.h"
#include <iostream>
#include <memory>

// Example 1: Load sprite sheet from JSON
void example_load_from_json() {
    std::cout << "Example 1: Loading sprite sheet from JSON\n";

    try {
        auto spriteSheet = SpriteSheet::loadFromJson("assets/sprites/example_idle.json");
        std::cout << "✓ Loaded sprite sheet with " << spriteSheet->getFrameCount() << " frames\n";

        // Access individual frames
        for (size_t i = 0; i < spriteSheet->getFrameCount(); ++i) {
            const auto& frame = spriteSheet->getFrame(i);
            std::cout << "  Frame " << i << ": " << frame.duration << "s duration\n";
        }
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << "\n";
    }
}

// Example 2: Create sprite sheet programmatically
void example_create_programmatically() {
    std::cout << "\nExample 2: Creating sprite sheet programmatically\n";

    try {
        auto spriteSheet = std::make_unique<SpriteSheet>("assets/characters/Annie.png");

        // Add frames for idle animation (4 frames)
        spriteSheet->addFrame("idle_1", sf::IntRect({0, 0}, {256, 512}), 0.15f);
        spriteSheet->addFrame("idle_2", sf::IntRect({256, 0}, {256, 512}), 0.15f);
        spriteSheet->addFrame("idle_3", sf::IntRect({512, 0}, {256, 512}), 0.15f);
        spriteSheet->addFrame("idle_4", sf::IntRect({768, 0}, {256, 512}), 0.15f);

        std::cout << "✓ Created sprite sheet with " << spriteSheet->getFrameCount() << " frames\n";

        // Access by name
        const auto& frame = spriteSheet->getFrameByName("idle_2");
        std::cout << "✓ Retrieved frame by name: idle_2\n";
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << "\n";
    }
}

// Example 3: Use with CharacterAnimator
void example_with_animator() {
    std::cout << "\nExample 3: Using sprite sheet with CharacterAnimator\n";

    try {
        // Load sprite sheet
        auto spriteSheet = SpriteSheet::loadFromJson("assets/sprites/example_idle.json");

        // Create animator
        CharacterAnimator animator("assets/characters/Annie.png", 256, 512);

        // Load animations from sprite sheet
        animator.loadAnimationFromSpriteSheet(CharacterExpression::IDLE, *spriteSheet, 0, 4);

        std::cout << "✓ Loaded IDLE animation with 4 frames\n";

        // Play animation
        animator.setExpression(CharacterExpression::IDLE);
        std::cout << "✓ Animation playing\n";

        // Simulate frame updates
        float totalTime = 0.0f;
        float deltaTime = 0.016f;  // ~60 FPS

        for (int i = 0; i < 10; ++i) {
            animator.update(deltaTime);
            totalTime += deltaTime;
            int frameIdx = animator.getCurrentFrameIndex();
            std::cout << "  Time: " << totalTime << "s, Frame: " << frameIdx << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << "\n";
    }
}

// Example 4: Grid-based loading
void example_grid_based() {
    std::cout << "\nExample 4: Grid-based frame loading\n";

    try {
        auto spriteSheet = std::make_unique<SpriteSheet>("assets/characters/Annie.png");

        // Load 16 frames from a 4×4 grid
        // Each frame is 256×512, arranged in rows of 4
        spriteSheet->addGridFrames(256, 512, 4, 0.1f);

        std::cout << "✓ Loaded " << spriteSheet->getFrameCount() << " frames from grid\n";

        // Access frames by index
        for (size_t i = 0; i < spriteSheet->getFrameCount(); ++i) {
            const auto& frame = spriteSheet->getFrame(i);
            const auto& rect = frame.textureRect;
            std::cout << "  Frame " << i << ": position (" << rect.position.x << ", "
                      << rect.position.y << ") size (" << rect.size.x << ", " << rect.size.y << ")\n";
        }
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << "\n";
    }
}

// Example 5: Multiple expressions per character
void example_multiple_expressions() {
    std::cout << "\nExample 5: Multiple expressions per character\n";

    try {
        // Load different sprite sheets for different expressions
        auto idle_sheet = SpriteSheet::loadFromJson("assets/sprites/example_idle.json");

        CharacterAnimator animator("assets/characters/Annie.png", 256, 512);

        // You would load different sprite sheets for each expression in a real app
        // For this example, we'll load the same sheet for all expressions
        animator.loadAnimationFromSpriteSheet(CharacterExpression::IDLE, *idle_sheet, 0, 4);
        animator.loadAnimationFromSpriteSheet(CharacterExpression::TALKING, *idle_sheet, 0, 4);
        animator.loadAnimationFromSpriteSheet(CharacterExpression::HAPPY, *idle_sheet, 0, 4);

        std::cout << "✓ Loaded 3 expression animations\n";

        // Switch between expressions
        std::cout << "Switching expressions:\n";
        animator.setExpression(CharacterExpression::IDLE);
        std::cout << "  IDLE: " << (int)animator.getCurrentExpression() << "\n";

        animator.setExpression(CharacterExpression::TALKING);
        std::cout << "  TALKING: " << (int)animator.getCurrentExpression() << "\n";

        animator.setExpression(CharacterExpression::HAPPY);
        std::cout << "  HAPPY: " << (int)animator.getCurrentExpression() << "\n";
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "=== Sprite Sheet Integration Examples ===\n\n";

    example_load_from_json();
    example_create_programmatically();
    example_with_animator();
    example_grid_based();
    example_multiple_expressions();

    std::cout << "\n=== Examples Complete ===\n";
    return 0;
}
