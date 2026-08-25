#include "../include/story_engine.h"
#include <stdexcept>

double GameState::getVar(const std::string& name, double defVal) const {
    auto it = variables.find(name);
    return (it != variables.end()) ? it->second : defVal;
}

void GameState::setVar(const std::string& name, double value) {
    variables[name] = value;
}

bool GameState::hasFlag(const std::string& name) const {
    return flags.find(name) != flags.end();
}

void GameState::setFlag(const std::string& name, bool value) {
    if (value) {
        flags.insert(name);
    } else {
        flags.erase(name);
    }
}

StoryEngine::StoryEngine(const Script& script)
    : script(script), currentNodeId("start") {
    onNodeLoaded();
}

const StoryNode& StoryEngine::getCurrentNode() const {
    auto it = script.nodes.find(currentNodeId);
    if (it == script.nodes.end()) {
        throw std::runtime_error("Current node not found: " + currentNodeId);
    }
    return it->second;
}

const std::string& StoryEngine::getCurrentNodeId() const {
    return currentNodeId;
}

void StoryEngine::jumpToNode(const std::string& nodeId) {
    if (script.nodes.find(nodeId) == script.nodes.end()) {
        throw std::runtime_error("Cannot jump to non-existent node: " + nodeId);
    }
    currentNodeId = nodeId;
    onNodeLoaded();
}

void StoryEngine::advanceNode() {
    const auto& node = getCurrentNode();
    if (node.next.empty()) {
        throw std::runtime_error("Cannot advance from node with no 'next'");
    }
    jumpToNode(node.next);
}

void StoryEngine::makeChoice(const std::string& choiceKey) {
    const auto& node = getCurrentNode();
    for (const auto& choice : node.choices) {
        if (choice.key == choiceKey) {
            gameState.lastChosenKey = choiceKey;
            gameState.choicesMade++;

            for (const auto& [varName, varVal] : choice.setVars) {
                gameState.setVar(varName, varVal);
            }

            for (const auto& flagName : choice.setFlags) {
                gameState.setFlag(flagName);
            }

            jumpToNode(choice.next);
            return;
        }
    }
    throw std::runtime_error("Choice key not found: " + choiceKey);
}

const GameState& StoryEngine::getGameState() const {
    return gameState;
}

GameState& StoryEngine::getMutableGameState() {
    return gameState;
}

void StoryEngine::updatePlaytime(float deltaSeconds) {
    gameState.playtimeSecs += deltaSeconds;
}

void StoryEngine::addToBacklog(const std::string& speaker, const std::string& text) {
    backlog.push_back({speaker, text});
    if (backlog.size() > 100) {
        backlog.erase(backlog.begin());
    }
}

const std::vector<BacklogEntry>& StoryEngine::getBacklog() const {
    return backlog;
}

void StoryEngine::onNodeLoaded() {
    gameState.nodesVisited.insert(currentNodeId);
}
