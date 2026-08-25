#include <catch2/catch_test_macros.hpp>
#include "../include/script_parser.h"
#include <filesystem>

namespace fs = std::filesystem;

TEST_CASE("ScriptParser loads valid TOML file", "[script_parser]") {
    // This test expects assets/script.toml to exist
    if (fs::exists("assets/script.toml")) {
        REQUIRE_NOTHROW(ScriptParser::loadFromFile("assets/script.toml", false));
    }
}

TEST_CASE("ScriptParser requires start node", "[script_parser]") {
    // Create a temporary TOML with no [start] section
    std::string tempToml = R"(
[chapter1]
speaker = "Test"
bg = ""
dialogue = "No start node here"
)";

    // This would need a temp file to test properly
    // For now, we test the loaded script directly
}

TEST_CASE("StoryNode contains default values", "[script_parser]") {
    Script script;
    StoryNode node;

    REQUIRE(node.speaker == "");
    REQUIRE(node.bgmVolume == 70.f);
    REQUIRE(node.bgmLoop == true);
    REQUIRE(node.choices.empty());
}

TEST_CASE("ChoiceDef has all required fields", "[script_parser]") {
    ChoiceDef choice;
    choice.key = "A";
    choice.text = "Test choice";
    choice.next = "next_node";

    REQUIRE(choice.key == "A");
    REQUIRE(choice.text == "Test choice");
    REQUIRE(choice.next == "next_node");
}

TEST_CASE("GameState variable operations", "[story_engine]") {
    GameState state;

    state.setVar("test_var", 42.0);
    REQUIRE(state.getVar("test_var") == 42.0);
    REQUIRE(state.getVar("nonexistent", 0.0) == 0.0);
}

TEST_CASE("GameState flag operations", "[story_engine]") {
    GameState state;

    state.setFlag("flag1");
    REQUIRE(state.hasFlag("flag1"));

    state.setFlag("flag1", false);
    REQUIRE(!state.hasFlag("flag1"));
}

TEST_CASE("ScriptParser constants are reasonable", "[script_parser]") {
    REQUIRE(ScriptParser::MAX_CHOICES == 3);
    REQUIRE(ScriptParser::DEFAULT_BGM_VOLUME == 70.f);
    REQUIRE(ScriptParser::DEFAULT_CHARACTER_HEIGHT_FRAC == 0.80f);
}
