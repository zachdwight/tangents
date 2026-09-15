# Changelog

All notable changes to Tangents will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.5.0] - 2026-09-15

### Added
- **Settings Menu System** — Complete in-game configuration interface
  - Press [Tab] to open settings overlay menu
  - Full keyboard navigation with ↑↓← → arrows
  - Settings Manager: persistent JSON-based configuration storage
  - Auto-save on menu close, auto-load on startup

### Settings Options
- **Audio:** BGM Volume (0-100%), SFX Volume (0-100%)
- **Display:** Fullscreen toggle, Resolution (1920x1080 default)
- **Gameplay:** Text Speed (0.5x, 1.0x, 2.0x), Auto-advance toggle
- **Visual:** Brightness (0.5-1.5), Screen Shake toggle

### UI Features
- Beautiful overlay menu with semi-transparent background
- Color-coded interface (yellow selection, cyan adjustments)
- Slider controls for continuous values (5% increments for volume)
- Binary toggles for on/off settings
- Multi-step selections for discrete options
- Reset to defaults functionality
- Clear on-screen instructions

### Technical
- SettingsManager: persistent JSON storage
- SettingsUI: full-featured in-game menu
- Automatic directory creation for config files
- Value validation and clamping on apply
- Simple but effective JSON parser
- ~600 lines of settings code

### Integration
- Seamless menu integration with existing UI
- Non-blocking: navigate away with Esc
- Persists across game sessions
- Config file: `config/settings.json`

## [0.4.0] - 2026-09-15

### Added
- **Character Animation System** — Frame-based sprite animation framework
  - Animation class for managing sprite frame sequences
  - CharacterAnimator for character-specific expressions
  - Six expression types: Idle, Talking, Happy, Sad, Shocked, Angry
  - Support for sprite sheets with configurable frame rectangles
  - Smooth animation playback with delta-time updates
  - Loop control and frame advancement
  - Foundation for relationship-based expression changes

- **Shader Effects System** — Transition and screen effect framework
  - Fade transitions (black/white) with configurable duration
  - Screen effects: brightness, color tint, blur simulation
  - Smooth easing (ease-in-out quadratic) for transitions
  - Dynamic effect intensity control (0.0-1.0 range)
  - SFML Shader support with software fallback
  - Ready for advanced effects (noise, distortion, etc.)

### Technical
- `Animation` class: Frame-based sprite animation sequencing
- `CharacterAnimator` class: Multi-expression character animation management
- `ShaderEffects` class: Transition and screen effect management
- Delta-time based updates for smooth motion
- SFML 3.x compatible vector-based rect construction
- Zero runtime cost when effects not in use

### Integration
- Animation and shader systems initialized at game start
- Shader transitions update in main game loop
- Foundation for relationship-to-expression mapping
- Prepared for sprite sheet integration

### Performance
- Frame advancement: 60+ FPS capable
- Transition timing: Precise frame-based easing
- Memory efficient: Only active animations consume CPU

## [0.3.0] - 2026-09-15

### Added
- **Relationship Meter UI** — Visual character affinity tracking system
  - Color gradient display: Red (hostile) → Yellow (neutral) → Green (devoted)
  - Five affinity levels: Hostile, Cold, Neutral, Friendly, Devoted
  - Shows percentage (0-100) and descriptive label
  - Positioned in top-right during gameplay
  - Integrated with game state save/load

### Technical
- Added `RelationshipUI` class for meter rendering
- Extended `GameState` with relationship tracking
- Methods: `getRelationship()`, `setRelationship()`, `modifyRelationship()`
- Color system: Red/Yellow/Green gradient based on affinity value
- Example setup with 3 characters (Alice, Bob, Charlie)

### Features
- Relationships persist across save/load
- Automatic meter layout based on character count
- No performance impact (reuses font cache)

## [0.2.0] - 2026-09-15

### Added
- **Save/Load Game State** — Full game persistence system
  - 10 save slots with automatic slot management
  - Serialized state: variables, flags, visited nodes, playtime, current node position
  - Save metadata: title, timestamp, playtime display
  - Press [S] to save, [L] to load
  - Visual distinction between empty and filled slots
  - Auto-save on major story progression (after choices/advances)
- **Save Manager UI**
  - Dedicated save and load menus with slot navigation
  - Up/Down arrows for slot selection
  - Enter to confirm, Escape to cancel
  - Shows save info including playtime and node name
- **JSON Persistence** — Simple, human-readable save file format

### Technical
- Added `SaveManager` class for state serialization/deserialization
- Added `GameUIState` enum for menu state management
- Save files stored in `saves/` directory
- Auto-save functionality integrated with story progression
- No external JSON library dependency (manual serialization)

## [0.1.1] - 2026-09-15

### Added
- **Font Caching** — Fonts now cached on startup instead of loading from disk every frame
- **Backlog/History UI** — Players can press [H] to view dialogue history (last 20 entries)
- On-screen hint displaying [H] key for accessing history

### Improved
- **Performance** — Eliminated ~60 font I/O operations per second
- Backlog automatically populated as story progresses
- Clean separation between normal gameplay and history view

### Technical
- Added `FontCache` class mirroring `TextureCache` architecture
- `StoryEngine::onNodeLoaded()` now auto-populates backlog
- Pre-load UI font at game startup

## [0.1.0] - 2026-08-25

### Added
- Initial release of Tangents visual novel engine
- C++20 modern codebase with SFML 3.x graphics/audio
- TOML-based script format for narrative design
- Game state management (variables, flags, inventory)
- Multi-character support with layered positioning and alpha blending
- Audio system with BGM streaming and SFX playback
- Modular architecture: parser, engine, rendering, audio
- Comprehensive documentation (README, DEVELOPMENT.md, STORY_GUIDE.md)
- Text wrapping and dialogue rendering with font support
- Choice system with keyboard input (A/B/C keys)
- Background and character sprite rendering
- Playtime tracking and node visitation history
- GitHub Actions CI/CD pipeline
- Code linting with clang-tidy and formatting with clang-format

### Features
- Node-based story structure with branching choices
- Visited node tracking and history
- Backlog system for dialogue tracking
- Variable and flag-based game state
- Texture and font caching

---

## Upcoming

### [0.5.0] - Planned
- Settings menu (volume, resolution, text speed, fullscreen)
- Dialogue speed customization
- Graphics options (quality, window size)
- Character expression automation (tie animations to relationships)
- Sprite sheet integration examples

### [0.6.0] - Planned
- Advanced shader effects (noise, distortion, pixelation)
- Character portrait animation sequences
- Scene transitions with custom shader effects
- Language/localization support
- Mobile platform support (iOS/Android)
