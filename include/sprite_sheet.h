#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

struct SpriteFrame {
    sf::IntRect textureRect;  // Position and size in sprite sheet
    float duration = 0.1f;     // Seconds to display this frame
    std::string name;          // Optional frame name
};

class SpriteSheet {
public:
    // Load sprite sheet from image file
    explicit SpriteSheet(const std::string& imagePath);

    // Add a frame to the sheet
    void addFrame(const std::string& name, const sf::IntRect& rect, float duration = 0.1f);

    // Add multiple frames (grid-based)
    void addGridFrames(int frameWidth, int frameHeight, int framesPerRow, float duration = 0.1f);

    // Get a specific frame by index
    const SpriteFrame& getFrame(size_t index) const;
    const SpriteFrame& getFrameByName(const std::string& name) const;

    // Get all frames for an animation
    const std::vector<SpriteFrame>& getFrames() const { return frames; }

    // Access texture
    const sf::Texture& getTexture() const { return texture; }
    sf::Texture& getMutableTexture() { return texture; }

    // Query
    size_t getFrameCount() const { return frames.size(); }
    bool isValid() const { return textureLoaded; }

    // Load from JSON definition
    static std::unique_ptr<SpriteSheet> loadFromJson(const std::string& jsonPath);

private:
    sf::Texture texture;
    std::vector<SpriteFrame> frames;
    std::unordered_map<std::string, size_t> frameMap;  // name -> index
    bool textureLoaded = false;

    void indexFrames();
};

// JSON Format:
// {
//   "image": "assets/characters/alice.png",
//   "frameWidth": 256,
//   "frameHeight": 512,
//   "frames": [
//     {"x": 0, "y": 0, "duration": 0.1, "name": "idle_1"},
//     {"x": 256, "y": 0, "duration": 0.1, "name": "idle_2"},
//     ...
//   ]
// }
//
// OR grid-based:
// {
//   "image": "assets/characters/alice.png",
//   "frameWidth": 256,
//   "frameHeight": 512,
//   "framesPerRow": 4,
//   "duration": 0.1
// }
