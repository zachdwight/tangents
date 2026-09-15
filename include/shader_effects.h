#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

enum class TransitionType {
    FADE,
    FADE_BLACK,
    FADE_WHITE,
    NONE
};

enum class ScreenEffect {
    NONE,
    BRIGHTNESS,
    COLOR_TINT,
    BLUR_SIMPLE
};

class ShaderEffects {
public:
    ShaderEffects();

    // Transition effects
    void startTransition(TransitionType type, float duration);
    void updateTransition(float deltaTime);
    bool isTransitioning() const { return transitioning; }
    float getTransitionAlpha() const { return transitionAlpha; }

    // Screen effects
    void applyScreenEffect(ScreenEffect effect, float intensity = 1.0f);
    void clearScreenEffect();

    // Apply effects to render target
    void applyEffects(sf::RenderWindow& window, const sf::Texture& screenTexture);

private:
    // Transition state
    bool transitioning = false;
    float transitionAlpha = 0.0f;
    float transitionDuration = 0.0f;
    float transitionElapsed = 0.0f;
    TransitionType currentTransition = TransitionType::NONE;
    sf::Color transitionColor = sf::Color::Black;

    // Screen effect state
    ScreenEffect currentEffect = ScreenEffect::NONE;
    float effectIntensity = 0.0f;

    // Shaders (if supported)
    std::unique_ptr<sf::Shader> brightnessShader;
    std::unique_ptr<sf::Shader> colorTintShader;
    std::unique_ptr<sf::Shader> blurShader;

    bool shadersSupported = false;

    void initializeShaders();
    void drawTransitionOverlay(sf::RenderWindow& window);
};
