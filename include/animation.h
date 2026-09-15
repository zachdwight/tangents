#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

class SpriteSheet;

struct AnimationFrame {
    sf::IntRect textureRect;  // Position in sprite sheet
    float duration;            // Seconds to display this frame
};

enum class CharacterExpression {
    IDLE,
    TALKING,
    HAPPY,
    SAD,
    SHOCKED,
    ANGRY
};

class Animation {
public:
    Animation();

    // Define animation from sprite sheet
    void addFrame(const sf::IntRect& rect, float duration);
    void setFrames(const std::vector<AnimationFrame>& frames);

    // Control animation
    void play(bool loop = true);
    void pause();
    void stop();
    void reset();

    // Update and rendering
    void update(float deltaTime);
    const AnimationFrame& getCurrentFrame() const;
    bool isFinished() const;

    // State queries
    bool isPlaying() const { return playing; }
    int getCurrentFrameIndex() const { return currentFrame; }
    float getElapsedTime() const { return elapsedTime; }

private:
    std::vector<AnimationFrame> frames;
    int currentFrame = 0;
    float elapsedTime = 0.0f;
    bool playing = false;
    bool looping = false;
};

class CharacterAnimator {
public:
    CharacterAnimator(const std::string& spriteSheetPath, float textureWidth, float textureHeight);

    // Define animations for this character
    void defineAnimation(CharacterExpression expression, const std::vector<AnimationFrame>& frames);

    // Load animation from SpriteSheet
    void loadAnimationFromSpriteSheet(CharacterExpression expression, const SpriteSheet& sheet, size_t startFrame, size_t frameCount);

    // Play animation by expression
    void setExpression(CharacterExpression expression, bool immediate = false);

    // Update and get current animation
    void update(float deltaTime);
    const AnimationFrame& getCurrentFrame() const;
    const sf::Texture& getTexture() const { return texture; }

    // Expression state
    CharacterExpression getCurrentExpression() const { return currentExpression; }
    bool isAnimationFinished() const { return currentAnimation.isFinished(); }

private:
    sf::Texture texture;
    std::unordered_map<int, Animation> animations;
    CharacterExpression currentExpression = CharacterExpression::IDLE;
    Animation currentAnimation;
    bool textureLoaded = false;

    int expressionToInt(CharacterExpression expr) const;
};
