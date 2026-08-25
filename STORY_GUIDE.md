# Story Creation Guide for Tangents

This guide shows how to write interactive stories for the Tangents visual novel engine using TOML script format.

## Table of Contents

1. [Quick Start](#quick-start)
2. [TOML Basics](#toml-basics)
3. [Story Structure](#story-structure)
4. [Game State Management](#game-state-management)
5. [Characters and Dialogue](#characters-and-dialogue)
6. [Branching Logic](#branching-logic)
7. [Audio Integration](#audio-integration)
8. [Advanced Features](#advanced-features)
9. [Complete Example](#complete-example)
10. [Best Practices](#best-practices)

---

## Quick Start

The simplest story has a title, at least one character, and a [start] node:

```toml
[game]
title = "My Story"
version = "0.1.0"
company = "Your Name"

[window]
width = 1920
height = 1080

[start]
speaker = "Narrator"
dialogue = "The adventure begins..."
next = "chapter1"

[chapter1]
speaker = "Hero"
dialogue = "Let's do this!"
```

Save as `assets/script.toml`, then run `make run`.

---

## TOML Basics

**TOML** (Tom's Obvious, Minimal Language) is a simple config format. Key points:

```toml
# Comments start with #

# Strings use quotes
name = "Alice"

# Numbers (integers or floats)
affection = 50.5

# Arrays use brackets
items = ["key", "sword", "potion"]

# Tables use [name]
[character]
name = "Bob"
level = 10
```

**For Tangents:**
- Each story node is a `[table]` (e.g., `[start]`, `[scene_1]`, `[ending_good]`)
- Top-level tables like `[game]`, `[window]`, `[fonts]` configure the game
- Node tables contain `speaker`, `dialogue`, `bg`, `char`, etc.

---

## Story Structure

### Basic Node

```toml
[my_scene]
speaker = "Character Name"
dialogue = "What the character says"
bg = "images/environments/forest.png"
char = "images/characters/hero.png"
next = "next_scene"
```

**Fields:**
- `speaker` - Character name (required, defaults to "Narrator")
- `dialogue` - Character's dialogue text (string or array of strings)
- `bg` - Background image path (optional)
- `char` - Character sprite path (optional, single character)
- `next` - Auto-advance to this node when dialogue finishes (optional)

### Multi-Page Dialogue

Use an array for multiple pages of text (click to advance each page):

```toml
[long_speech]
speaker = "Philosopher"
dialogue = [
  "First page of dialogue...",
  "Second page of dialogue...",
  "Third page of dialogue..."
]
next = "next_scene"
```

---

## Game State Management

### Variables

Track numeric values like affection, gold, or stats:

```toml
[initial_state]
variables = {annie_affection = 0, gold = 100, health = 100}
```

Access in choices to branch logic:

```toml
[choice_scene]
speaker = "Annie"
dialogue = "Do you like me?"

[[choice_scene.choices]]
key = "A"
text = "Yes, absolutely!"
next = "annie_happy"
set_vars = {annie_affection = 100}

[[choice_scene.choices]]
key = "B"
text = "Not really..."
next = "annie_sad"
set_vars = {annie_affection = 0}
```

### Flags

Track boolean conditions (true/false):

```toml
[initial_state]
flags = []

[get_key_scene]
speaker = "Narrator"
dialogue = "You found a key!"
next = "hallway"
set_flags = ["has_key"]

[locked_door]
speaker = "Narrator"
dialogue = "A locked door. You need a key."
# (Engine can check: if has_key flag, unlock automatically)
```

---

## Characters and Dialogue

### Single Character

```toml
[scene_alice]
speaker = "Alice"
dialogue = "Hello!"
char = "images/characters/alice.png"
bg = "images/environments/park.png"
```

### Multiple Characters

Overlay multiple character sprites with positioning:

```toml
[meeting]
speaker = "Alice"
dialogue = "Hey Bob, how are you?"
bg = "images/environments/cafe.png"

[[meeting.chars]]
file = "images/characters/alice.png"
pos = "left"          # left, center, or right
height = 0.80         # 80% of screen height
x = 0                 # X offset in pixels
y = 0                 # Y offset in pixels
z = 1                 # Draw order (higher = on top)
alpha = 1.0           # Opacity (0 = invisible, 1 = opaque)

[[meeting.chars]]
file = "images/characters/bob.png"
pos = "right"
height = 0.80
z = 0
```

**Positioning:**
- `pos`: "left" (0), "center" (0.5), "right" (1), or numeric 0.0-1.0
- `height`: Character height as fraction of screen (0.50 = half height)
- `x`, `y`: Pixel offsets from anchored position
- `z`: Draw order (lower values render first, higher values on top)
- `alpha`: 0 (invisible) to 1 (fully visible)

### Speaker Changes

```toml
[dialogue_exchange]
speaker = "Alice"
dialogue = "What do you want?"

[bob_responds]
speaker = "Bob"
dialogue = "Just saying hi!"
next = "alice_smiles"

[alice_smiles]
speaker = "Alice"
dialogue = "Oh, nice!"
```

---

## Branching Logic

### Simple Choices

```toml
[fork_in_road]
speaker = "Narrator"
dialogue = "Left or right?"

[[fork_in_road.choices]]
key = "A"
text = "Take the left path"
next = "left_path"

[[fork_in_road.choices]]
key = "B"
text = "Take the right path"
next = "right_path"

[[fork_in_road.choices]]
key = "C"
text = "Wait and think"
next = "fork_in_road"
```

### Conditional Choices

Show different text based on game state (affection, inventory, etc.):

```toml
[confession_scene]
speaker = "Love Interest"
dialogue = "Do you have feelings for me?"

[[confession_scene.choices]]
key = "A"
text = "I love you"
next = "confession_yes"
conditions = ["affection >= 80"]     # Only show if affection is 80+
set_vars = {affection = 100}

[[confession_scene.choices]]
key = "B"
text = "Let's just be friends"
next = "confession_friend"
set_vars = {affection = 50}

[[confession_scene.choices]]
key = "C"
text = "Leave them"
next = "ending_lonely"
```

**Conditions:**
- `>`, `>=`, `<`, `<=`, `==`, `!=` operators
- Examples: `"gold >= 50"`, `"has_key"`, `"health < 20"`
- Conditions that evaluate to false are hidden from the player

### State Changes

```toml
[[fork_in_road.choices]]
key = "A"
text = "Go left"
next = "left_path"
set_vars = {current_path = 1, steps_taken = 5}
set_flags = ["visited_left_path"]
```

---

## Audio Integration

### Background Music

Set music for the current scene:

```toml
[peaceful_scene]
speaker = "Narrator"
dialogue = "It's a calm day..."
bg = "images/environments/forest.png"
bgm = "assets/sfx/bgm/peaceful_loop.wav"
bgm_loop = true
bgm_volume = 70
```

**Fields:**
- `bgm` - Path to audio file (WAV, OGG, FLAC)
- `bgm_loop` - Repeat when finished (true/false)
- `bgm_volume` - Volume 0-100 (default 70)

**To stop music:**
```toml
[silent_scene]
bgm = ""  # Empty string stops current music
```

### Sound Effects

Play a sound when a choice is selected:

```toml
[[choice_scene.choices]]
key = "A"
text = "Pick up the sword"
next = "have_sword"
sfx = "assets/sfx/clicks/sfx_confirm.wav"
```

---

## Advanced Features

### Relationship Meters

Track affection for multiple characters:

```toml
[meta]
girls = ["Alice", "Bob", "Charlie"]

[initial_state]
variables = {
  alice_affection = 0,
  bob_affection = 0,
  charlie_affection = 0
}
```

Later in story:
```toml
[[choice_scene.choices]]
text = "Give flowers to Alice"
next = "next_scene"
set_vars = {alice_affection = 25}

[[choice_scene.choices]]
text = "Give flowers to Bob"
next = "next_scene"
set_vars = {bob_affection = 25}
```

### Inventory System

Track collected items:

```toml
[initial_state]
variables = {gold = 0}

[found_treasure]
speaker = "Narrator"
dialogue = "You found 50 gold coins!"
next = "continue"
set_vars = {gold = 50}

[shop_scene]
speaker = "Shopkeeper"
dialogue = "That'll be 30 gold"

[[shop_scene.choices]]
key = "A"
text = "Buy item"
next = "purchased"
conditions = ["gold >= 30"]
set_vars = {gold = 20}
set_flags = ["has_item"]

[[shop_scene.choices]]
key = "B"
text = "Can't afford it"
next = "shop_scene"
```

### Chapter Markers

Organize story into chapters:

```toml
[meta]
chapter_thresholds = [6, 14, 22, 30]  # Scenes where chapters end

[chapter_1_intro]
speaker = "Narrator"
dialogue = "CHAPTER 1: The Beginning"

[chapter_2_intro]
speaker = "Narrator"
dialogue = "CHAPTER 2: The Plot Thickens"
```

---

## Complete Example

Here's a short dating sim story:

```toml
[game]
title = "Coffee Date"
version = "0.1.0"
company = "Your Name"

[window]
width = 1920
height = 1080

[meta]
girls = ["Alex"]

[initial_state]
variables = {alex_affection = 0}
flags = []

[start]
speaker = "Narrator"
dialogue = "You're meeting Alex at the coffee shop..."
bg = "images/environments/cafe.png"
next = "cafe_arrival"

[cafe_arrival]
speaker = "Alex"
dialogue = "Hey! Great to see you!"
char = "images/characters/alex.png"
bgm = "assets/sfx/bgm/cafe_loop.wav"
bgm_loop = true
bgm_volume = 60

[[cafe_arrival.choices]]
key = "A"
text = "You look amazing today!"
next = "compliment_response"
set_vars = {alex_affection = 50}

[[cafe_arrival.choices]]
key = "B"
text = "How have you been?"
next = "neutral_response"
set_vars = {alex_affection = 25}

[[cafe_arrival.choices]]
key = "C"
text = "Sorry I'm late"
next = "apology_response"
set_vars = {alex_affection = 10}

[compliment_response]
speaker = "Alex"
dialogue = [
  "Wow, thank you! That's so sweet!",
  "I really like spending time with you..."
]
char = "images/characters/alex_happy.png"
next = "ending_good"

[neutral_response]
speaker = "Alex"
dialogue = "Pretty good! Just work stuff, you know."
char = "images/characters/alex.png"
next = "ending_neutral"

[apology_response]
speaker = "Alex"
dialogue = "Don't worry about it! I just got here anyway."
char = "images/characters/alex.png"
next = "ending_neutral"

[ending_good]
speaker = "Narrator"
dialogue = [
  "You spent a wonderful afternoon together.",
  "Alex gave you their number at the end."
]
conditions = ["alex_affection >= 50"]
next = "credits"

[ending_neutral]
speaker = "Narrator"
dialogue = "You had a nice time, but nothing special happened."
next = "credits"

[credits]
speaker = "Narrator"
dialogue = "Thanks for playing Coffee Date!"
bgm = ""
```

---

## Best Practices

### 1. Node Naming

Use clear, descriptive names:

✅ **Good:**
- `chapter1_meeting_alice`
- `affection_50_confession`
- `ending_good`

❌ **Bad:**
- `scene1`, `scene2`, `scene3`
- `a`, `b`, `c`
- `xxx`

### 2. Organization

Group related scenes with prefixes:

```toml
# Chapter 1 scenes
[ch1_intro]
[ch1_alice_meeting]
[ch1_choice_response]

# Chapter 2 scenes
[ch2_intro]
[ch2_plot_twist]
[ch2_climax]

# Endings
[ending_good]
[ending_neutral]
[ending_bad]
```

### 3. Comments

Use comments to explain complex logic:

```toml
# This branch only shows if player has high affection
# AND has collected the special item
[[choice_scene.choices]]
key = "A"
text = "Confess with the gift"
next = "confession_success"
conditions = ["affection >= 80", "has_gift"]
set_vars = {affection = 100}
```

### 4. File Paths

Always use relative paths from repo root:

✅ **Good:**
```toml
bg = "assets/images/environments/forest.png"
char = "assets/characters/alice.png"
bgm = "assets/sfx/bgm/peaceful_loop.wav"
```

❌ **Bad:**
```toml
bg = "/Users/you/tangents/assets/images/..."
bg = "../images/forest.png"
```

### 5. Dialogue Length

Keep dialogue readable (wrap at 80-100 characters):

✅ **Good:**
```toml
dialogue = [
  "This is a long line of dialogue that wraps to",
  "the next page for readability."
]
```

❌ **Bad:**
```toml
dialogue = "This is a very long line of dialogue that would be hard to read on screen because it goes on and on and on without breaking..."
```

### 6. Variable Naming

Use snake_case for consistency:

✅ **Good:**
- `alice_affection`
- `gold_count`
- `has_key`
- `visited_forest`

❌ **Bad:**
- `AliceAffection`
- `goldCount`
- `hasKey`
- `VisitedForest`

### 7. Choice Design

Always provide an exit strategy:

```toml
[[choice_scene.choices]]
key = "A"
text = "Continue"
next = "next_scene"

[[choice_scene.choices]]
key = "B"
text = "Go back"
next = "previous_scene"
```

### 8. Testing

Create a test branch to verify story flow:

```toml
# At the end of script.toml
[test_ending_good]
speaker = "Narrator"
dialogue = "Test: Ending Good"
conditions = ["test_mode"]

[test_ending_bad]
speaker = "Narrator"
dialogue = "Test: Ending Bad"
conditions = ["test_mode"]
```

---

## Debugging Tips

### Verify Asset Files

If images or audio don't load:
1. Check file paths are relative to repo root
2. Verify files actually exist: `ls assets/images/characters/`
3. Check file format (PNG/JPG for images, WAV/OGG for audio)

### Check Node References

If game crashes when clicking a choice:
1. Verify `next` node exists in script
2. Look for typos in node names (case-sensitive!)
3. Example: `[cafe_scene]` vs `[cafe_scene_2]` — both valid but different

### Validate TOML Syntax

Use an online TOML validator:
- https://www.toml-lint.com/
- Paste your script.toml to check for syntax errors

### Enable Debug Output

Build with debug info:
```bash
make clean
make build
./tangents 2>&1 | grep -i "error\|warning"
```

---

## Examples by Story Type

### Choice-Heavy Story (Visual Novel)

Focus on branching with many choices:
- Use multiple choice nodes
- Track affection for each character
- Have different endings based on final values
- Keep dialogue per scene short (1-3 lines)

### Story-Heavy Novel (Text Adventure)

Focus on narrative with fewer choices:
- Use long dialogue arrays for pages
- Minimize choices (maybe 1-2 key decisions)
- Use flags for plot progression
- Long exposition scenes between major choices

### Puzzle Game with Story

Mix story and logic:
- Track variables for puzzle state
- Use conditions to block paths until puzzle solved
- Celebrate player victories with positive dialogue
- Provide hints through character dialogue

---

## Next Steps

1. **Create your story** in `assets/script.toml`
2. **Test it**: `make run` and play through all branches
3. **Add art**: Create or download character sprites and backgrounds
4. **Add music**: Record or download background music and sound effects
5. **Polish**: Tweak affection values, add more dialogue, create more endings
6. **Share**: Push to GitHub, share with friends!

---

## Getting Help

- **TOML syntax**: https://toml.io/
- **Story structure**: See DEVELOPMENT.md for architecture
- **Code changes**: See CONTRIBUTING.md for contributing guidelines
- **Issues**: Check GitHub Issues for known problems
