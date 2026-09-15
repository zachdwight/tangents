#pragma once

#include "story_engine.h"
#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

struct SaveFile {
    int slot;
    std::string title;
    std::string timestamp;
    double playtimeSecs;
    std::string currentNode;
    bool exists;
};

class SaveManager {
public:
    explicit SaveManager(const std::string& savesDirectory = "saves");

    // Save/Load operations
    void saveGame(const StoryEngine& engine, int slot, const std::string& title);
    bool loadGame(StoryEngine& engine, int slot);

    // Save file management
    std::vector<SaveFile> listSaves() const;
    SaveFile getSaveInfo(int slot) const;
    bool deleteSave(int slot) const;
    bool autosave(const StoryEngine& engine);

private:
    fs::path savesDir;
    static constexpr int MAX_SLOTS = 10;
    static constexpr const char* AUTOSAVE_FILE = "autosave.json";

    std::string serializeGameState(const GameState& state) const;
    bool deserializeGameState(GameState& state, const std::string& json) const;

    std::string getCurrentTimestamp() const;
    fs::path getSlotPath(int slot) const;
};
