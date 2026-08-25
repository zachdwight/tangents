# Tangents Development Guide

## Architecture Overview

Tangents uses a **modular, layered architecture** separating concerns:

```
┌─────────────────────────────────┐
│     main.cpp (SFML Loop)        │  Window, event handling, rendering
└────────────┬────────────────────┘
             │
     ┌───────┴────────┬──────────┐
     │                │          │
┌────▼──────┐  ┌─────▼─────┐  ┌─▼───────────┐
│UIRenderer │  │AudioManager│  │StoryEngine  │  Game logic, flow
└────▲──────┘  └─────▲─────┘  └─▲───────────┘
     │                │          │
     │                └──────────┴──────────┐
     │                                      │
     └──────────────────┬───────────────────┘
                        │
                    ┌───▼────────────┐
                    │ ScriptParser   │   TOML parsing, validation
                    └────────────────┘
                            │
                        ┌───▼──────────┐
                        │ assets/      │   Script, images, audio
                        │ script.toml  │
                        └──────────────┘
```

## Module Responsibilities

### ScriptParser (`include/script_parser.h`, `src/script_parser.cpp`)

**Purpose**: Load and validate TOML story scripts

**Key Classes**:
- `ScriptParser` (static) - Entry point for loading TOML files

**Key Structs**:
- `Script` - Collection of story nodes
- `StoryNode` - Single narrative point (dialogue, choices, assets)
- `ChoiceDef` - Player choice with target node
- `CharacterDef` - Character sprite with positioning

**Validation**:
- File existence (images, audio files)
- Choice key validity (A/B/C, max 3)
- Node reference integrity (no broken links)
- Required [start] node

**Constants**:
- `MAX_CHOICES = 3` - Maximum choices per node
- `DEFAULT_CHARACTER_HEIGHT_FRAC = 0.80f` - Default character height
- `DEFAULT_BGM_VOLUME = 70.f` - Default background music volume

### StoryEngine (`include/story_engine.h`, `src/story_engine.cpp`)

**Purpose**: Manage game state and narrative flow

**Key Classes**:
- `StoryEngine` - Orchestrates node navigation and state updates
- `GameState` - Player progress (variables, flags, inventory)

**Game State Fields**:
- `variables` - Numeric tracking (affection, stats, counters)
- `flags` - Boolean conditions (talked_to_annie, has_key)
- `inventory` - String-based item list
- `playtimeSecs` - Total time elapsed
- `nodesVisited` - Set of all nodes player has seen

**Engine Operations**:
- `jumpToNode(id)` - Direct navigation
- `advanceNode()` - Auto-advance via "next" field
- `makeChoice(key)` - Handle player choice, update state, transition
- `updatePlaytime(deltaSeconds)` - Track elapsed time

**State Side-effects**:
- When a choice is made: set variables, set flags, increment choicesMade
- When a node is loaded: add to nodesVisited

### UIRenderer (`include/ui_renderer.h`, `src/ui_renderer.cpp`)

**Purpose**: Render text, sprites, and manage textures

**Key Classes**:
- `UIRenderer` - Text wrapping, sprite scaling
- `TextureCache` - Load and cache sprite textures (prevent duplicates)

**Rendering Methods**:
- `setWrappedText()` - Wrap text to max width with word-break
- `scaleSpriteCover()` - Fill screen (letterbox black bars on edges)
- `scaleCharacterBottom()` - Scale to height fraction, bottom-align
- `placeCharacterBottom()` - Positioned character with anchor (0=left, 0.5=center, 1=right)

**Texture Caching**:
- Loads texture once, reuses from map
- Called by main loop each frame to avoid reloads

### AudioManager (`include/audio_manager.h`, `src/audio_manager.cpp`)

**Purpose**: Stream BGM and play SFX effects

**Key Classes**:
- `AudioManager` - BGM and SFX lifecycle
- `SoundCache` - Load and cache SFX sound buffers

**Audio Methods**:
- `playBGM(path, loop, volume)` - Start music (streamed)
- `stopBGM()`, `pauseBGM()`, `resumeBGM()` - BGM control
- `playSFX(path, volume)` - One-shot sound (from cache)
- `stopAllSFX()` - Clear all active sounds

**Lifecycle**:
- BGM: One active `sf::Music` (streamed file)
- SFX: Vector of `sf::Sound` (shared buffers from cache)
- Cleanup: Remove finished sounds from vector each frame

## Data Flow

### Game Load

```
1. main() initializes SFML window
2. ScriptParser::loadFromFile("assets/script.toml")
   ├─ Parse TOML structure
   ├─ Validate node references
   ├─ Validate asset files
   └─ Return Script (map of StoryNode)
3. StoryEngine engine(script)
   └─ Initialize at "start" node
4. Enter main loop
```

### Game Loop (Each Frame)

```
1. Input handling (keyboard, mouse)
   ├─ Spacebar → advanceNode()
   ├─ A/B/C → makeChoice("A"|"B"|"C")
   └─ Escape → quit
2. Update (dt = delta time)
   └─ engine.updatePlaytime(dt)
3. Render
   ├─ Draw background texture
   ├─ Draw character sprites (sorted by z-index)
   ├─ Render dialogue text (with wrapping)
   └─ Draw choice buttons
4. Present frame
```

### Choice Execution

```
player presses 'A'
    ↓
StoryEngine::makeChoice("A")
    ├─ Find choice with key "A"
    ├─ For each variable in choice.setVars:
    │   └─ gameState.setVar(varName, value)
    ├─ For each flag in choice.setFlags:
    │   └─ gameState.setFlag(flagName)
    ├─ Increment choicesMade
    ├─ jumpToNode(choice.next)
    │   ├─ Update currentNodeId
    │   └─ onNodeLoaded() → add to nodesVisited
    └─ return
```

## Extension Points

### Adding a Feature

**Example: Save/Load System**

1. Create `src/save_system.h/cpp`
2. Implement: `SaveSystem::save(engine.getGameState()) → file`
3. Implement: `SaveSystem::load(file) → GameState`
4. Call from main.cpp on Ctrl+S / Ctrl+L
5. Write tests in `tests/test_save_system.cpp`
6. Update CI/CD

**Example: Relationship Meter UI**

1. Create `src/relationship_ui.h/cpp`
2. Add to UIRenderer or separate class
3. Track affection in GameState.variables
4. Render in main loop: draw bars for each character
5. Update when choice.setVars sets affection value

**Example: Character Animation**

1. Extend `StoryNode::CharacterDef` with animation state
2. Update TOML format with frame info
3. Modify UIRenderer to cycle between sprite frames
4. Update parser validation for animation files

### Modifying Script Format

**Example: Add background effects**

1. Add field to `StoryNode`: `std::string bgEffect = "none"`
2. Parse in `ScriptParser::loadFromFile()`:
   ```cpp
   n.bgEffect = node->get("bg_effect") ? ... : "none";
   ```
3. Validate effect names in parser
4. Apply effect in UIRenderer (shade, blur, etc.)
5. Update documentation and examples

## Performance Considerations

### Texture Caching
- Textures are large (images can be 10+ MB)
- Cache prevents reloading same image every frame
- Clear cache on scene change if needed

### Audio Streaming
- BGM uses `sf::Music` (streams from disk)
- SFX use `sf::Sound` + `SoundBuffer` (fully loaded)
- Keep SFX files small (<1 MB each)
- Consider music compression (OGG/FLAC)

### Text Rendering
- Text wrapping is CPU-bound (word-split algorithm)
- Cache wrapped text if it doesn't change
- Avoid updating text every frame

### Memory
- Typical project: <100 MB (code + assets)
- script.nodes map: O(n) where n = node count
- Each texture: varies (usually 2-10 MB)
- Each sound buffer: varies (usually 0.1-2 MB)

## Testing Strategy

### Unit Tests (Catch2)

**Parser Tests**: 
- Valid TOML loading
- Invalid choice keys
- Missing asset files
- Broken node references

**Engine Tests**:
- State variable operations
- Flag state transitions
- Node navigation
- Playtime updates

**UI Tests** (harder to test without graphics):
- Text wrapping logic
- Sprite scaling math
- Position calculations

### Integration Tests

Test end-to-end flows:
- Load script → make choice A → verify node changed
- Load script → make choice with setVars → verify variable set
- Playtime after 5 seconds elapsed

### Manual Testing

1. Load full script.toml
2. Play through all branches (exhaustive playthrough)
3. Check all audio/images load correctly
4. Test UI layout on 1920x1080

## Debugging

### Enable Verbose Logging

In `script_parser.cpp`, `story_engine.cpp`:
```cpp
std::cout << "DEBUG: Loaded node " << nodeId << "\n";
std::cout << "DEBUG: Player chose: " << choiceKey << "\n";
```

### Common Issues

**Audio not playing**:
- Check file exists and is in assets/sfx/
- Verify AudioManager was initialized
- Check volume isn't 0

**Text doesn't render**:
- Font file missing
- Max width too narrow
- Text color same as background

**Texture not showing**:
- File path wrong (relative to working directory)
- Image format not supported (use PNG/JPG)
- Texture too large (scale down if > 4096x4096)

## Build Details

### Compiler Flags
- `-std=c++20` - C++ version
- `-O2` - Optimization level (speed vs build time)
- `-Wall -Wextra -Wpedantic` - Warning levels

### Dependencies
- SFML 3.x (graphics, audio, windowing)
- toml++ (header-only TOML parser)
- Catch2 (optional, testing)

### Linking
- `-lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system`
- Order matters (on some systems)

## Future Improvements

- [ ] Character animation support (sprite sheets)
- [ ] Dialogue history/backlog UI
- [ ] Save/load game state
- [ ] Input rebinding UI
- [ ] Visual effects (shaders, transitions)
- [ ] Mobile platform support (touch input)
- [ ] Voice acting integration
- [ ] Branching story metrics/analytics
