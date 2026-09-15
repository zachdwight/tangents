# Changelog

All notable changes to Tangents will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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

### [0.4.0] - Planned
- Character animation support (sprite animation frames and sequences)
- Shader effects (transitions, screen filters, fade effects)
- Character expressions tied to dialogue/relationships

### [0.5.0] - Planned
- Settings menu (volume, resolution, text speed, fullscreen)
- Dialogue speed customization
- Graphics options (quality, window size)
- Language/localization support
- Mobile platform support (iOS/Android)
