#include "sprite_sheet.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdexcept>

SpriteSheet::SpriteSheet(const std::string& imagePath) {
    if (!texture.loadFromFile(imagePath)) {
        throw std::runtime_error("Failed to load sprite sheet: " + imagePath);
    }
    textureLoaded = true;
}

void SpriteSheet::addFrame(const std::string& name, const sf::IntRect& rect, float duration) {
    SpriteFrame frame;
    frame.name = name;
    frame.textureRect = rect;
    frame.duration = duration;
    frames.push_back(frame);
    indexFrames();
}

void SpriteSheet::addGridFrames(int frameWidth, int frameHeight, int framesPerRow, float duration) {
    const auto& texSize = texture.getSize();
    int cols = framesPerRow;
    int rows = static_cast<int>(std::ceil(static_cast<float>(texSize.y) / frameHeight));

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int x = col * frameWidth;
            int y = row * frameHeight;

            // Stop if we're beyond texture bounds
            if (x + frameWidth > static_cast<int>(texSize.x) || y + frameHeight > static_cast<int>(texSize.y)) {
                return;
            }

            SpriteFrame frame;
            frame.textureRect = sf::IntRect({x, y}, {frameWidth, frameHeight});
            frame.duration = duration;
            frame.name = "frame_" + std::to_string(row * cols + col);
            frames.push_back(frame);
        }
    }
    indexFrames();
}

const SpriteFrame& SpriteSheet::getFrame(size_t index) const {
    if (index >= frames.size()) {
        throw std::out_of_range("Frame index out of range");
    }
    return frames[index];
}

const SpriteFrame& SpriteSheet::getFrameByName(const std::string& name) const {
    auto it = frameMap.find(name);
    if (it == frameMap.end()) {
        throw std::runtime_error("Frame not found: " + name);
    }
    return frames[it->second];
}

void SpriteSheet::indexFrames() {
    frameMap.clear();
    for (size_t i = 0; i < frames.size(); ++i) {
        if (!frames[i].name.empty()) {
            frameMap[frames[i].name] = i;
        }
    }
}

std::unique_ptr<SpriteSheet> SpriteSheet::loadFromJson(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open JSON file: " + jsonPath);
    }

    std::string line;
    std::string imagePath;
    int frameWidth = 0;
    int frameHeight = 0;
    int framesPerRow = 0;
    float duration = 0.1f;
    bool useGridMode = false;

    // Parse JSON manually (simple format)
    while (std::getline(file, line)) {
        // Extract "image": "path"
        if (line.find("\"image\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':') + 1) + 1;
            size_t end = line.find('"', start);
            imagePath = line.substr(start, end - start);
        }
        // Extract "frameWidth": value
        if (line.find("\"frameWidth\"") != std::string::npos) {
            size_t pos = line.find(':');
            frameWidth = std::stoi(line.substr(pos + 1));
        }
        // Extract "frameHeight": value
        if (line.find("\"frameHeight\"") != std::string::npos) {
            size_t pos = line.find(':');
            frameHeight = std::stoi(line.substr(pos + 1));
        }
        // Extract "framesPerRow": value (indicates grid mode)
        if (line.find("\"framesPerRow\"") != std::string::npos) {
            size_t pos = line.find(':');
            framesPerRow = std::stoi(line.substr(pos + 1));
            useGridMode = true;
        }
        // Extract "duration": value
        if (line.find("\"duration\"") != std::string::npos) {
            size_t pos = line.find(':');
            duration = std::stof(line.substr(pos + 1));
        }
    }

    if (imagePath.empty()) {
        throw std::runtime_error("Missing 'image' field in JSON: " + jsonPath);
    }
    if (frameWidth <= 0 || frameHeight <= 0) {
        throw std::runtime_error("Invalid frameWidth or frameHeight in JSON: " + jsonPath);
    }

    auto sheet = std::make_unique<SpriteSheet>(imagePath);

    if (useGridMode && framesPerRow > 0) {
        sheet->addGridFrames(frameWidth, frameHeight, framesPerRow, duration);
    } else {
        // TODO: Parse individual frame definitions from JSON array
        // For now, just use grid mode as fallback
        sheet->addGridFrames(frameWidth, frameHeight, 1, duration);
    }

    return sheet;
}
