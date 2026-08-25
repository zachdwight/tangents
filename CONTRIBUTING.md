# Contributing to Tangents

We welcome contributions! Here's how to get started.

## Code Style

### Language & Standards
- **C++ Standard**: C++20 (no C++17 features)
- **Formatter**: clang-format (run `make fmt`)
- **Linter**: clang-tidy (run `make lint`)

### Style Rules

1. **Naming**
   - Classes: PascalCase (e.g., `ScriptParser`, `GameState`)
   - Functions/methods: camelCase (e.g., `loadFromFile()`, `makeChoice()`)
   - Variables: snake_case (e.g., `current_node_id`, `bgm_volume`)
   - Constants: UPPER_SNAKE_CASE (e.g., `MAX_CHOICES`, `DEFAULT_BGM_VOLUME`)
   - Private members: prefixed with `m_` if needed, or use public interface

2. **Indentation**
   - 4 spaces (no tabs)
   - Braces on new line (Allman style)

3. **Line Length**
   - Maximum 100 characters
   - Break long lines at logical points

4. **Comments**
   - One-line comments only (`//`)
   - Only explain *why*, not *what* (code should be self-documenting)
   - Avoid comments for obvious code

5. **Memory**
   - Prefer stack allocation and smart pointers
   - Use `std::unique_ptr` for exclusive ownership
   - Use `std::shared_ptr` only when necessary
   - No raw `new`/`delete`

### Example

```cpp
class StoryEngine
{
public:
    explicit StoryEngine(const Script& script);

    const StoryNode& getCurrentNode() const;
    void jumpToNode(const std::string& nodeId);

private:
    const Script& script;
    std::string currentNodeId;
    GameState gameState;
};
```

## Testing

### Adding Tests

1. Add test cases to `tests/test_script_parser.cpp`
2. Use Catch2 test macros:

```cpp
TEST_CASE("Description of what is tested", "[module]") {
    REQUIRE(expected == actual);
    REQUIRE_THROWS(someFunction());
}
```

3. Run tests: `make test`

### Test Coverage Goals
- Parser: All validation paths
- Engine: State transitions, edge cases
- Renderer: Scaling/positioning logic
- Audio: Cache and lifecycle management

## Pull Request Process

1. **Create a branch**: `git checkout -b feature/your-feature-name`
2. **Make changes**: Implement your feature
3. **Format code**: `make fmt`
4. **Run checks**: `make all` (builds, lints, tests)
5. **Commit**:
   ```bash
   git add .
   git commit -m "Add feature: description"
   ```
6. **Push**: `git push origin feature/your-feature-name`
7. **Create PR** on GitHub with:
   - Clear title and description
   - Link to any related issues
   - Test summary

## Commit Messages

Format:
```
[scope]: Short description (50 chars max)

Longer explanation if needed (72 chars max per line).
Explain the WHY, not the WHAT.

Fixes #123
```

Examples:
- `parser: Validate file paths at load time`
- `engine: Add playtime tracking to GameState`
- `ui: Fix text wrapping edge case with hyphens`

## Common Tasks

### Adding a New Module

1. Create header in `include/`
2. Create implementation in `src/`
3. Add to `CMakeLists.txt` and `Makefile`
4. Add to `src/main.cpp` includes
5. Write tests in `tests/`
6. Update `.github/workflows/ci.yml` if needed

### Modifying Script Format

1. Update TOML parsing in `script_parser.cpp`
2. Update data structures in `script_parser.h`
3. Update validation logic
4. Add test cases
5. Update `README.md` script format section

### Performance Improvements

1. Profile with Chrome DevTools (if applicable) or `perf`
2. Add comments explaining the optimization
3. Include benchmark results in PR description
4. Ensure no correctness regressions

## Bug Reports

File an issue with:
- **Title**: Clear, concise description
- **Steps to reproduce**: Exact sequence
- **Expected behavior**: What should happen
- **Actual behavior**: What actually happens
- **System info**: macOS version, Xcode version, etc.

Example:
```
Title: Audio cuts out when making rapid choices

Steps:
1. Load game
2. Make 5+ choices within 2 seconds
3. Observe audio glitch

Expected: Audio continues smoothly
Actual: Audio stutters/cuts out
```

## Feature Requests

Describe:
- **Use case**: Why is this needed?
- **Solution**: How should it work?
- **Alternatives**: Other approaches considered
- **Impact**: Backwards-compatible?

## Getting Help

- Check existing issues/PRs
- Read architecture docs in `DEVELOPMENT.md`
- Ask in PR reviews
- Email project maintainer

## Code Review Checklist

When reviewing, check:
- [ ] Follows code style (run `make fmt` + `make lint`)
- [ ] Tests added for new functionality
- [ ] No unnecessary dependencies
- [ ] Comments explain non-obvious logic
- [ ] Commit messages are clear
- [ ] No hardcoded paths or magic numbers

## Release Process

(Maintainers only)

1. Update version in `CMakeLists.txt` and `include/script_parser.h`
2. Update `CHANGELOG.md`
3. Tag: `git tag -a v0.2.0 -m "Release 0.2.0"`
4. Push tags: `git push origin --tags`
5. Create GitHub Release with changelog
