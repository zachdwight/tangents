# Tangents — Visual Novel Engine

A modern C++20 visual novel framework with modular architecture, SFML rendering, and TOML-based scripting.

## Features

- **Modern C++20** with modular, testable code
- **SFML 3.x** for graphics, audio, and windowing
- **TOML Script Format** for narrative design (no hardcoding)
- **Game State Management** with variables, flags, and inventory tracking
- **Multi-character Support** with layered positioning and alpha blending
- **Audio System** with BGM streaming and SFX playback
- **Modular Architecture** separated into: parser, engine, rendering, audio
- **Comprehensive Testing** with Catch2 unit tests
- **Linting & Formatting** with clang-tidy and clang-format

## Quick Start

### Prerequisites

```bash
brew install sfml llvm catch2 clang-format
```

### Build

```bash
cd ~/code_projects/tangents

# Build executable
make build

# Run game
make run

# Run all checks (build, lint, test)
make all
```

### Development

```bash
# Format code
make fmt

# Run linter
make lint

# Run unit tests
make test

# Clean build artifacts
make clean
```

## Project Structure

```
tangents/
├── src/
│   ├── main.cpp              # Entry point, SFML window loop
│   ├── script_parser.cpp     # TOML loading & validation
│   ├── story_engine.cpp      # Game state & flow logic
│   ├── ui_renderer.cpp       # Text & sprite rendering
│   └── audio_manager.cpp     # BGM/SFX playback
│
├── include/
│   ├── script_parser.h       # ScriptParser class, data structures
│   ├── story_engine.h        # StoryEngine, GameState classes
│   ├── ui_renderer.h         # UIRenderer, TextureCache classes
│   └── audio_manager.h       # AudioManager, SoundCache classes
│
├── tests/
│   └── test_script_parser.cpp # Catch2 unit tests
│
├── assets/
│   ├── script.toml           # Story script (TOML format)
│   ├── fonts/                # .ttf font files
│   ├── images/               # Background & character sprites
│   └── sfx/                  # Audio files
│
├── CMakeLists.txt            # CMake build (alternative to Makefile)
├── Makefile                  # GNU Make build system
├── .clang-tidy               # Linting configuration
├── .clang-format             # Code formatting rules
└── .github/workflows/ci.yml  # GitHub Actions CI/CD
```

## Script Format (TOML)

Define your story in `assets/script.toml`:

```toml
[game]
title = "Tangents"
version = "0.1.0"
company = "Vine Street Labs"

[window]
width = 1920
height = 1080

[start]
speaker = "Narrator"
bg = "images/environments/Diner.png"
dialogue = "Welcome to Tangents..."
[[start.choices]]
key = "A"
text = "Choice A text"
next = "node_a"

[[start.choices]]
key = "B"
text = "Choice B text"
next = "node_b"

[node_a]
speaker = "Character A"
dialogue = "You chose A!"
next = "end"

[node_b]
speaker = "Character B"
dialogue = "You chose B!"
next = "end"

[end]
speaker = "Narrator"
dialogue = "The end!"
```

## Architecture

### ScriptParser
Loads and validates TOML scripts. Checks for:
- Missing asset files
- Invalid choice keys (A/B/C max)
- Broken node references
- Required [start] node

### StoryEngine
Manages game flow and state:
- Node navigation (jump, advance, make choice)
- Variable/flag tracking
- Playtime tracking
- Visited node history

### UIRenderer
Handles SFML rendering:
- Text wrapping to max width
- Sprite scaling (cover/character modes)
- Character positioning with anchors
- Texture caching

### AudioManager
Manages sound playback:
- BGM streaming (loopable, volume control)
- SFX playback (cached buffers)
- Sound lifecycle management

## Testing

Unit tests with Catch2:

```bash
make test
```

Tests cover:
- Script parsing and validation
- Game state operations
- Data structure initialization

Add new tests to `tests/test_script_parser.cpp`.

## Linting

Code style enforcement with clang-tidy:

```bash
make lint
```

Configuration in `.clang-tidy` enables:
- readability rules (naming, function complexity)
- performance rules (unnecessary copies, inefficient containers)
- modernize rules (C++20 features, deprecated APIs)
- bugprone rules (common mistakes, undefined behavior)

## Formatting

Auto-format code with clang-format:

```bash
make fmt
```

Rules defined in `.clang-format` enforce:
- 4-space indentation
- Allman braces
- 100-column line limit
- Pointer alignment (left)

## GitHub Actions CI

Automatically on every push/PR:
1. ✅ Build with clang++
2. ✅ Run linter (clang-tidy)
3. ✅ Run unit tests (Catch2)
4. ✅ Check formatting (clang-format)

See `.github/workflows/ci.yml`.

## CMake Alternative

For larger projects, use CMake:

```bash
mkdir build && cd build
cmake ..
make
make test   # Run tests
make lint   # Run linter
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Code style guidelines
- How to add features
- Testing requirements
- Commit message format

## Development Guide

See [DEVELOPMENT.md](DEVELOPMENT.md) for:
- Architecture deep-dive
- How to add a new module
- Extending the script format
- Performance considerations

## License

MIT License - See LICENSE file

## Roadmap

- [ ] Save/load game state
- [ ] Relationship meter UI
- [ ] Character animation support
- [ ] Shader effects (transitions, filters)
- [ ] Mobile platform support
- [ ] Dialogue history UI
