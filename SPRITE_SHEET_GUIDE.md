# Sprite Sheet Integration Guide

The Tangents engine now supports sprite sheet loading and frame-based character animation through the **SpriteSheet** and **CharacterAnimator** systems.

## Quick Start

### 1. Create a Sprite Sheet

Prepare your character sprite sheet as a PNG image with evenly-spaced frames arranged in a grid.

Example layout:
- **Frame size:** 256×512 pixels (width × height)
- **Grid arrangement:** 4 frames per row
- **Total frames:** 8 frames (2 rows)

### 2. Define Frame Locations (JSON)

Create a JSON configuration file describing your sprite sheet:

```json
{
  "image": "assets/characters/alice.png",
  "frameWidth": 256,
  "frameHeight": 512,
  "framesPerRow": 4,
  "duration": 0.15
}
```

**Fields:**
- `image` — Path to sprite sheet PNG file
- `frameWidth` — Width of each frame in pixels
- `frameHeight` — Height of each frame in pixels
- `framesPerRow` — Frames per horizontal row (for grid parsing)
- `duration` — Default frame display duration in seconds

### 3. Load in Code

```cpp
#include "sprite_sheet.h"
#include "animation.h"

// Load sprite sheet from JSON
auto spriteSheet = SpriteSheet::loadFromJson("assets/sprites/example_idle.json");

// Create character animator
CharacterAnimator animator("assets/characters/alice.png", 256, 512);

// Load animation frames from sprite sheet
// loadAnimationFromSpriteSheet(expression, spriteSheet, startFrame, frameCount)
animator.loadAnimationFromSpriteSheet(
    CharacterExpression::IDLE,
    *spriteSheet,
    0,  // Start at frame 0
    4   // Use 4 frames
);

// Play the animation
animator.setExpression(CharacterExpression::IDLE);

// Update in game loop
animator.update(deltaTime);

// Get current frame for rendering
const auto& frame = animator.getCurrentFrame();
// Use frame.textureRect with SFML sprite rendering
```

## Sprite Sheet Layout Examples

### Grid-Based (Recommended)

Simple uniform grid layout — best for consistent animations.

```
[Frame 0] [Frame 1] [Frame 2] [Frame 3]
[Frame 4] [Frame 5] [Frame 6] [Frame 7]
```

**JSON config:**
```json
{
  "image": "assets/characters/alice.png",
  "frameWidth": 256,
  "frameHeight": 512,
  "framesPerRow": 4,
  "duration": 0.1
}
```

### Manual Frame Definition (Advanced)

For non-uniform layouts, manually define each frame's position:

```cpp
auto sheet = std::make_unique<SpriteSheet>("assets/characters/alice.png");

// Add individual frames
sheet->addFrame("idle_1", sf::IntRect({0, 0}, {256, 512}), 0.15f);
sheet->addFrame("idle_2", sf::IntRect({256, 0}, {256, 512}), 0.15f);
sheet->addFrame("idle_3", sf::IntRect({512, 0}, {256, 512}), 0.15f);
sheet->addFrame("idle_4", sf::IntRect({768, 0}, {256, 512}), 0.15f);
```

## Character Expression Types

The engine supports 6 expression types for character animation:

```cpp
enum class CharacterExpression {
    IDLE,      // Default neutral state
    TALKING,   // Mouth moving, dialogue
    HAPPY,     // Smiling, joyful
    SAD,       // Sad, disappointed
    SHOCKED,   // Surprised, shocked
    ANGRY      // Angry, confrontational
};
```

## Complete Example

```cpp
#include "sprite_sheet.h"
#include "animation.h"

// Load sprite sheet
auto spriteSheet = SpriteSheet::loadFromJson("assets/sprites/alice_idle.json");

// Create animator
CharacterAnimator alice("assets/characters/alice.png", 256, 512);

// Define animations using sprite sheet frames
alice.loadAnimationFromSpriteSheet(CharacterExpression::IDLE, *spriteSheet, 0, 4);
alice.loadAnimationFromSpriteSheet(CharacterExpression::TALKING, *spriteSheet, 4, 4);

// In game loop
alice.update(deltaTime);
alice.setExpression(CharacterExpression::TALKING);

// Get frame for rendering
const auto& frame = alice.getCurrentFrame();
// Use frame.textureRect to render the sprite
```

## Performance Notes

- **Frame caching:** Frames are cached in memory; no per-frame file I/O
- **Texture sharing:** Multiple characters can reference the same sprite sheet
- **Memory efficient:** Only active animations consume CPU cycles
- **Looping:** Character animations loop by default

## Sprite Sheet Preparation Tips

1. **Consistent frame size** — All frames must be the same width/height
2. **Grid alignment** — Ensure frames align to your grid perfectly
3. **Alpha transparency** — Use PNG with transparency for character cutouts
4. **No padding** — Frames should touch or be separated by consistent spacing
5. **Layer separation** — Keep UI elements on separate layers from character sprites

## Troubleshooting

**Issue: "Failed to load sprite sheet"**
- Verify the image path is correct and file exists
- Ensure image is in PNG format

**Issue: Frames are offset or misaligned**
- Check `frameWidth` and `frameHeight` match your actual sprite dimensions
- Verify `framesPerRow` is correct for your layout

**Issue: Animation doesn't play**
- Ensure `duration` values are positive (e.g., 0.1 for 100ms per frame)
- Call `animator.setExpression()` to activate an expression
- Call `animator.update(deltaTime)` in your game loop

## API Reference

### SpriteSheet Class

```cpp
class SpriteSheet {
    // Load from image file
    explicit SpriteSheet(const std::string& imagePath);

    // Add single frame
    void addFrame(const std::string& name, const sf::IntRect& rect, float duration);

    // Add grid of frames
    void addGridFrames(int frameWidth, int frameHeight, int framesPerRow, float duration);

    // Get frame by index or name
    const SpriteFrame& getFrame(size_t index) const;
    const SpriteFrame& getFrameByName(const std::string& name) const;

    // Query
    size_t getFrameCount() const;
    bool isValid() const;
    const sf::Texture& getTexture() const;

    // Load from JSON
    static std::unique_ptr<SpriteSheet> loadFromJson(const std::string& jsonPath);
};
```

### CharacterAnimator Integration

```cpp
// Load animation from sprite sheet frames
void loadAnimationFromSpriteSheet(
    CharacterExpression expression,
    const SpriteSheet& sheet,
    size_t startFrame,
    size_t frameCount
);
```

## Next Steps

- Create sprite sheets for your characters
- Define JSON configurations for each expression state
- Integrate with relationship system for automatic expression changes
- Combine with shader effects for advanced transitions
