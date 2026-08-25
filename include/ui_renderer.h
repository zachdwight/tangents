#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <vector>

class TextureCache {
public:
    sf::Texture& get(const std::string& path);
    void clear();

private:
    std::map<std::string, sf::Texture> textures;
};

class UIRenderer {
public:
    explicit UIRenderer(float screenWidth, float screenHeight);

    void setWrappedText(sf::Text& text, const std::string& str, float maxWidthPx);

    void scaleSpriteCover(sf::Sprite& spr, const sf::Texture& tex, float w, float h);
    void scaleCharacterBottom(
        sf::Sprite& spr,
        const sf::Texture& tex,
        float w,
        float h,
        float heightFrac = 0.80f);

    void placeCharacterBottom(
        sf::Sprite& spr,
        const sf::Texture& tex,
        float w,
        float h,
        float heightFrac,
        float anchor01,
        float xOffsetPx,
        float yOffsetPx);

    TextureCache& getTextureCache();

private:
    TextureCache textureCache;

    static std::vector<std::string> splitWords(const std::string& s);
};
