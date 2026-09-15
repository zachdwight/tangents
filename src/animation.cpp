#include "../include/animation.h"
#include "../include/sprite_sheet.h"
#include <iostream>

// Animation class
Animation::Animation() = default;

void Animation::addFrame(const sf::IntRect& rect, float duration) {
    frames.push_back({rect, duration});
}

void Animation::setFrames(const std::vector<AnimationFrame>& newFrames) {
    frames = newFrames;
    reset();
}

void Animation::play(bool loop) {
    playing = true;
    looping = loop;
    if (frames.empty()) {
        playing = false;
    }
}

void Animation::pause() {
    playing = false;
}

void Animation::stop() {
    playing = false;
    currentFrame = 0;
    elapsedTime = 0.0f;
}

void Animation::reset() {
    currentFrame = 0;
    elapsedTime = 0.0f;
    playing = false;
}

void Animation::update(float deltaTime) {
    if (!playing || frames.empty()) return;

    elapsedTime += deltaTime;

    // Advance frame if duration exceeded
    while (elapsedTime >= frames[currentFrame].duration) {
        elapsedTime -= frames[currentFrame].duration;
        currentFrame++;

        if (currentFrame >= static_cast<int>(frames.size())) {
            if (looping) {
                currentFrame = 0;
            } else {
                currentFrame = frames.size() - 1;
                playing = false;
            }
        }
    }
}

const AnimationFrame& Animation::getCurrentFrame() const {
    if (frames.empty()) {
        static AnimationFrame empty{sf::IntRect({0, 0}, {0, 0}), 0.0f};
        return empty;
    }
    return frames[currentFrame];
}

bool Animation::isFinished() const {
    return !playing && currentFrame >= static_cast<int>(frames.size()) - 1;
}

// CharacterAnimator class
CharacterAnimator::CharacterAnimator(const std::string& spriteSheetPath,
                                     float textureWidth, float textureHeight) {
    if (!texture.loadFromFile(spriteSheetPath)) {
        std::cerr << "Failed to load sprite sheet: " << spriteSheetPath << "\n";
        return;
    }
    textureLoaded = true;

    // Initialize animations with empty frames (will be defined later)
    for (int i = 0; i < 6; ++i) {
        animations[i] = Animation();
    }

    // Set default idle animation (simple 1-frame placeholder)
    animations[expressionToInt(CharacterExpression::IDLE)].addFrame(
        sf::IntRect({0, 0}, {static_cast<int>(textureWidth), static_cast<int>(textureHeight)}), 1.0f);
}

int CharacterAnimator::expressionToInt(CharacterExpression expr) const {
    return static_cast<int>(expr);
}

void CharacterAnimator::defineAnimation(CharacterExpression expression,
                                       const std::vector<AnimationFrame>& frames) {
    int index = expressionToInt(expression);
    if (animations.find(index) != animations.end()) {
        animations[index].setFrames(frames);
    }
}

void CharacterAnimator::loadAnimationFromSpriteSheet(CharacterExpression expression,
                                                     const SpriteSheet& sheet,
                                                     size_t startFrame,
                                                     size_t frameCount) {
    std::vector<AnimationFrame> frames;
    for (size_t i = 0; i < frameCount; ++i) {
        if (startFrame + i >= sheet.getFrameCount()) break;
        const auto& spriteFrame = sheet.getFrame(startFrame + i);
        frames.push_back({spriteFrame.textureRect, spriteFrame.duration});
    }
    defineAnimation(expression, frames);
}

void CharacterAnimator::setExpression(CharacterExpression expression, bool immediate) {
    if (currentExpression == expression && !immediate) return;

    currentExpression = expression;
    int index = expressionToInt(expression);

    if (animations.find(index) != animations.end()) {
        currentAnimation = animations[index];
        currentAnimation.play(true);  // Loop character animations
    }
}

void CharacterAnimator::update(float deltaTime) {
    if (textureLoaded) {
        currentAnimation.update(deltaTime);
    }
}

const AnimationFrame& CharacterAnimator::getCurrentFrame() const {
    return currentAnimation.getCurrentFrame();
}
