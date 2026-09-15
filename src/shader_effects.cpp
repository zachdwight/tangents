#include "../include/shader_effects.h"
#include <iostream>
#include <cmath>

ShaderEffects::ShaderEffects() {
    initializeShaders();
}

void ShaderEffects::initializeShaders() {
    // Check if shaders are supported
    shadersSupported = sf::Shader::isAvailable();

    if (!shadersSupported) {
        std::cout << "⚠ Shaders not supported on this system, using fallback effects\n";
        return;
    }

    // Note: Full shader implementation would go here
    // For now, we use simple software-based effects
}

void ShaderEffects::startTransition(TransitionType type, float duration) {
    transitioning = true;
    currentTransition = type;
    transitionDuration = duration;
    transitionElapsed = 0.0f;
    transitionAlpha = 0.0f;

    // Set transition color based on type
    switch (type) {
        case TransitionType::FADE:
        case TransitionType::FADE_BLACK:
            transitionColor = sf::Color::Black;
            break;
        case TransitionType::FADE_WHITE:
            transitionColor = sf::Color::White;
            break;
        case TransitionType::NONE:
            transitioning = false;
            break;
    }
}

void ShaderEffects::updateTransition(float deltaTime) {
    if (!transitioning) return;

    transitionElapsed += deltaTime;

    if (transitionElapsed >= transitionDuration) {
        transitionAlpha = 255.0f;
        transitioning = false;
    } else {
        // Smooth fade using easing function
        float progress = transitionElapsed / transitionDuration;
        // Ease-in-out quadratic
        float eased = progress < 0.5f ?
            2.0f * progress * progress :
            1.0f - std::pow(-2.0f * progress + 2.0f, 2.0f) / 2.0f;
        transitionAlpha = eased * 255.0f;
    }
}

void ShaderEffects::applyScreenEffect(ScreenEffect effect, float intensity) {
    currentEffect = effect;
    effectIntensity = std::clamp(intensity, 0.0f, 1.0f);
}

void ShaderEffects::clearScreenEffect() {
    currentEffect = ScreenEffect::NONE;
    effectIntensity = 0.0f;
}

void ShaderEffects::drawTransitionOverlay(sf::RenderWindow& window) {
    if (!transitioning && transitionAlpha <= 0.0f) return;

    sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
    sf::Color overlayColor = transitionColor;
    overlayColor.a = static_cast<unsigned char>(transitionAlpha);

    overlay.setFillColor(overlayColor);
    window.draw(overlay);
}

void ShaderEffects::applyEffects(sf::RenderWindow& window, [[maybe_unused]] const sf::Texture& screenTexture) {
    // Draw transition overlay
    drawTransitionOverlay(window);

    // Apply screen effects (brightness, tint, etc.)
    switch (currentEffect) {
        case ScreenEffect::BRIGHTNESS: {
            // Darken or brighten by drawing semi-transparent overlay
            sf::RectangleShape effect(sf::Vector2f(window.getSize()));
            sf::Color effectColor = effectIntensity > 0.5f ?
                sf::Color::White :  // Brighten
                sf::Color::Black;   // Darken
            float alpha = std::abs(effectIntensity - 0.5f) * 2.0f * 100.0f;
            effectColor.a = static_cast<unsigned char>(std::clamp(alpha, 0.0f, 255.0f));
            effect.setFillColor(effectColor);
            window.draw(effect);
            break;
        }

        case ScreenEffect::COLOR_TINT: {
            // Color tint overlay (red, blue, green based on intensity)
            sf::RectangleShape tint(sf::Vector2f(window.getSize()));
            sf::Color tintColor(255, 200, 200);  // Red tint example
            tintColor.a = static_cast<unsigned char>(effectIntensity * 100.0f);
            tint.setFillColor(tintColor);
            window.draw(tint);
            break;
        }

        case ScreenEffect::BLUR_SIMPLE: {
            // Simple blur simulation via darkening
            sf::RectangleShape blur(sf::Vector2f(window.getSize()));
            blur.setFillColor(sf::Color(100, 100, 100, static_cast<unsigned char>(effectIntensity * 50.0f)));
            window.draw(blur);
            break;
        }

        case ScreenEffect::NONE:
        default:
            break;
    }
}
