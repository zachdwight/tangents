#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <toml++/toml.h>

struct ChoiceDef {
    std::string key;
    std::string text;
    std::string next;
    std::string sfx;
    std::vector<std::string> conditions;
    std::unordered_map<std::string, double> setVars;
    std::vector<std::string> setFlags;
};

struct StoryNode {
    std::string speaker;
    std::string dialogue;
    std::vector<std::string> descriptionPages;
    std::vector<std::string> pages;
    std::string bgFile;
    std::string charFile;

    struct CharacterDef {
        std::string file;
        std::string pos = "center";
        float heightFrac = 0.80f;
        float xOffset = 0.f;
        float yOffset = 0.f;
        int z = 0;
        float alpha = 1.f;
    };

    std::vector<CharacterDef> chars;
    std::vector<ChoiceDef> choices;
    std::string next;
    std::string bgm;
    bool bgmLoop = true;
    float bgmVolume = 70.f;
};

struct Script {
    std::unordered_map<std::string, StoryNode> nodes;
};

class ScriptParser {
public:
    static constexpr float DEFAULT_CHARACTER_HEIGHT_FRAC = 0.80f;
    static constexpr float DEFAULT_BGM_VOLUME = 70.f;
    static constexpr float DEFAULT_TITLE_BGM_VOLUME = 55.f;
    static constexpr int MAX_CHOICES = 3;

    static Script loadFromFile(const std::string& path, bool validateAssets = true);

private:
    static void validateChoiceKey(const std::string& key, const std::string& context);
    static void validateFileExists(const std::string& path, const std::string& context);
    static void parseDialogueAndDescription(
        const toml::table& t,
        const std::string& context,
        std::string& outDialogue,
        std::vector<std::string>& outDescription);
    static std::vector<std::string> readPagesLegacy(const toml::table& t, const std::string& context);
};
