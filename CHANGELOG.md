# Changelog

All notable changes to Tangents will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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

### [0.2.0] - Planned
- Save/load game state functionality
- Relationship meter UI system
- Character animation support
- Shader effects (transitions, screen filters)
- Mobile platform support (iOS/Android)
- Enhanced dialogue history with pagination
- Settings menu (volume, resolution, etc.)
