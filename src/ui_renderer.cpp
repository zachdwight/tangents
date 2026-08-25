#include "../include/ui_renderer.h"
#include <cctype>
#include <stdexcept>

sf::Texture& TextureCache::get(const std::string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) return it->second;

    sf::Texture tex;
    if (!tex.loadFromFile(path)) {
        throw std::runtime_error("Failed to load texture: " + path);
    }
    auto ins = textures.emplace(path, std::move(tex));
    return ins.first->second;
}

void TextureCache::clear() {
    textures.clear();
}

UIRenderer::UIRenderer(float /* screenWidth */, float /* screenHeight */) {}

std::vector<std::string> UIRenderer::splitWords(const std::string& s) {
    std::vector<std::string> words;
    std::string cur;
    for (char c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!cur.empty()) {
                words.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) words.push_back(cur);
    return words;
}

void UIRenderer::setWrappedText(sf::Text& text, const std::string& str, float maxWidthPx) {
    std::string out;
    std::string paragraph;

    for (size_t i = 0; i <= str.size(); i++) {
        char c = (i < str.size()) ? str[i] : '\n';
        if (c == '\n') {
            const auto words = splitWords(paragraph);
            std::string line;

            for (size_t wi = 0; wi < words.size(); wi++) {
                std::string candidate = line.empty() ? words[wi] : (line + " " + words[wi]);
                text.setString(candidate);

                if (text.getLocalBounds().size.x > maxWidthPx && !line.empty()) {
                    out += line + "\n";
                    line = words[wi];
                } else {
                    line = candidate;
                }
            }

            out += line;
            out += "\n";
            paragraph.clear();
        } else {
            paragraph.push_back(c);
        }
    }

    if (!out.empty() && out.back() == '\n') out.pop_back();
    text.setString(out);
}

void UIRenderer::scaleSpriteCover(sf::Sprite& spr, const sf::Texture& tex, float w, float h) {
    auto ts = tex.getSize();
    if (!ts.x || !ts.y) return;

    float sx = w / static_cast<float>(ts.x);
    float sy = h / static_cast<float>(ts.y);
    float s = std::max(sx, sy);

    spr.setScale({s, s});

    float drawnW = static_cast<float>(ts.x) * s;
    float drawnH = static_cast<float>(ts.y) * s;
    spr.setPosition(sf::Vector2f((w - drawnW) / 2.f, (h - drawnH) / 2.f));
}

void UIRenderer::scaleCharacterBottom(
    sf::Sprite& spr,
    const sf::Texture& tex,
    float w,
    float h,
    float heightFrac) {
    auto ts = tex.getSize();
    if (!ts.x || !ts.y) return;

    float targetHeight = h * heightFrac;
    float s = targetHeight / static_cast<float>(ts.y);

    spr.setScale({s, s});
    sf::FloatRect b = spr.getGlobalBounds();
    spr.setPosition(sf::Vector2f((w - b.size.x) / 2.f, h - b.size.y));
}

void UIRenderer::placeCharacterBottom(
    sf::Sprite& spr,
    const sf::Texture& tex,
    float w,
    float h,
    float heightFrac,
    float anchor01,
    float xOffsetPx,
    float yOffsetPx) {
    auto ts = tex.getSize();
    if (!ts.x || !ts.y) return;

    float targetHeight = h * heightFrac;
    float s = targetHeight / static_cast<float>(ts.y);
    spr.setScale({s, s});

    sf::FloatRect b = spr.getGlobalBounds();

    float xCenter = std::clamp(anchor01, 0.f, 1.f) * w;
    float x = xCenter - (b.size.x / 2.f) + xOffsetPx;
    float y = (h - b.size.y) + yOffsetPx;
    spr.setPosition(sf::Vector2f(x, y));
}

TextureCache& UIRenderer::getTextureCache() {
    return textureCache;
}
