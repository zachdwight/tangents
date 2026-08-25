#pragma once

#include "script_parser.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>

struct GameState {
    std::unordered_map<std::string, double> variables;
    std::unordered_set<std::string> flags;
    std::vector<std::string> inventory;
    double playtimeSecs = 0.0;
    std::string lastChosenKey;
    std::unordered_set<std::string> nodesVisited;
    int choicesMade = 0;

    double getVar(const std::string& name, double defVal = 0.0) const;
    void setVar(const std::string& name, double value);
    bool hasFlag(const std::string& name) const;
    void setFlag(const std::string& name, bool value = true);
};

struct GameConfig {
    std::string title = "Default Title";
    std::string version = "0.0.0";
    std::string company = "Default Company";
    unsigned width = 1920;
    unsigned height = 1080;
    std::string uiFont = "fonts/Roboto_Condensed-Regular.ttf";
    std::string titleBg = "";
    std::string titleLogo = "";
    std::string startButtonText = "Start";
    std::string hintText = "Click Start or press Enter";
    std::string titleBgm = "";
    bool titleBgmLoop = true;
    float titleBgmVolume = 55.f;
};

struct BacklogEntry {
    std::string speaker;
    std::string text;
};

class StoryEngine {
public:
    explicit StoryEngine(const Script& script);

    const StoryNode& getCurrentNode() const;
    const std::string& getCurrentNodeId() const;

    void jumpToNode(const std::string& nodeId);
    void advanceNode();
    void makeChoice(const std::string& choiceKey);

    const GameState& getGameState() const;
    GameState& getMutableGameState();

    void updatePlaytime(float deltaSeconds);

    void addToBacklog(const std::string& speaker, const std::string& text);
    const std::vector<BacklogEntry>& getBacklog() const;

private:
    const Script& script;
    std::string currentNodeId;
    GameState gameState;
    std::vector<BacklogEntry> backlog;

    void onNodeLoaded();
};
