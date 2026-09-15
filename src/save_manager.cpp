#include "../include/save_manager.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <iostream>

SaveManager::SaveManager(const std::string& savesDirectory)
    : savesDir(savesDirectory) {
    if (!fs::exists(savesDir)) {
        fs::create_directories(savesDir);
    }
}

fs::path SaveManager::getSlotPath(int slot) const {
    return savesDir / ("save_" + std::to_string(slot) + ".json");
}

std::string SaveManager::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string SaveManager::serializeGameState(const GameState& state) const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"version\": \"0.1.1\",\n";
    json << "  \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
    json << "  \"playtimeSecs\": " << state.playtimeSecs << ",\n";
    json << "  \"choicesMade\": " << state.choicesMade << ",\n";
    json << "  \"lastChosenKey\": \"" << state.lastChosenKey << "\",\n";

    // Serialize variables
    json << "  \"variables\": {\n";
    bool firstVar = true;
    for (const auto& [name, value] : state.variables) {
        if (!firstVar) json << ",\n";
        json << "    \"" << name << "\": " << value;
        firstVar = false;
    }
    json << "\n  },\n";

    // Serialize flags
    json << "  \"flags\": [\n";
    bool firstFlag = true;
    for (const auto& flag : state.flags) {
        if (!firstFlag) json << ",\n";
        json << "    \"" << flag << "\"";
        firstFlag = false;
    }
    json << "\n  ],\n";

    // Serialize visited nodes
    json << "  \"nodesVisited\": [\n";
    bool firstNode = true;
    for (const auto& node : state.nodesVisited) {
        if (!firstNode) json << ",\n";
        json << "    \"" << node << "\"";
        firstNode = false;
    }
    json << "\n  ]\n";
    json << "}\n";

    return json.str();
}

bool SaveManager::deserializeGameState(GameState& state, const std::string& json) const {
    try {
        // Simple JSON parser for game state
        state.variables.clear();
        state.flags.clear();
        state.nodesVisited.clear();

        // Extract playtimeSecs
        size_t pos = json.find("\"playtimeSecs\": ");
        if (pos != std::string::npos) {
            pos += 16;
            size_t endPos = json.find(',', pos);
            state.playtimeSecs = std::stod(json.substr(pos, endPos - pos));
        }

        // Extract choicesMade
        pos = json.find("\"choicesMade\": ");
        if (pos != std::string::npos) {
            pos += 15;
            size_t endPos = json.find(',', pos);
            state.choicesMade = std::stoi(json.substr(pos, endPos - pos));
        }

        // Extract lastChosenKey
        pos = json.find("\"lastChosenKey\": \"");
        if (pos != std::string::npos) {
            pos += 18;
            size_t endPos = json.find('"', pos);
            state.lastChosenKey = json.substr(pos, endPos - pos);
        }

        // Extract variables
        pos = json.find("\"variables\": {");
        if (pos != std::string::npos) {
            pos += 14;
            size_t endPos = json.find('}', pos);
            std::string varsStr = json.substr(pos, endPos - pos);

            size_t varPos = 0;
            while ((varPos = varsStr.find('"', varPos)) != std::string::npos) {
                varPos++;
                size_t varNameEnd = varsStr.find('"', varPos);
                if (varNameEnd == std::string::npos) break;

                std::string varName = varsStr.substr(varPos, varNameEnd - varPos);
                varPos = varNameEnd + 1;

                size_t colonPos = varsStr.find(':', varPos);
                if (colonPos == std::string::npos) break;

                size_t commaPos = varsStr.find(',', colonPos);
                if (commaPos == std::string::npos) commaPos = varsStr.length();

                std::string valueStr = varsStr.substr(colonPos + 1, commaPos - colonPos - 1);
                // Trim whitespace
                valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);

                try {
                    state.variables[varName] = std::stod(valueStr);
                } catch (...) {
                    // Skip invalid values
                }

                varPos = commaPos + 1;
            }
        }

        // Extract flags
        pos = json.find("\"flags\": [");
        if (pos != std::string::npos) {
            pos += 10;
            size_t endPos = json.find(']', pos);
            std::string flagsStr = json.substr(pos, endPos - pos);

            size_t flagPos = 0;
            while ((flagPos = flagsStr.find('"', flagPos)) != std::string::npos) {
                flagPos++;
                size_t flagEnd = flagsStr.find('"', flagPos);
                if (flagEnd == std::string::npos) break;

                std::string flag = flagsStr.substr(flagPos, flagEnd - flagPos);
                if (!flag.empty()) {
                    state.flags.insert(flag);
                }

                flagPos = flagEnd + 1;
            }
        }

        // Extract nodesVisited
        pos = json.find("\"nodesVisited\": [");
        if (pos != std::string::npos) {
            pos += 18;
            size_t endPos = json.find(']', pos);
            std::string nodesStr = json.substr(pos, endPos - pos);

            size_t nodePos = 0;
            while ((nodePos = nodesStr.find('"', nodePos)) != std::string::npos) {
                nodePos++;
                size_t nodeEnd = nodesStr.find('"', nodePos);
                if (nodeEnd == std::string::npos) break;

                std::string node = nodesStr.substr(nodePos, nodeEnd - nodePos);
                if (!node.empty()) {
                    state.nodesVisited.insert(node);
                }

                nodePos = nodeEnd + 1;
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to deserialize game state: " << e.what() << "\n";
        return false;
    }
}

void SaveManager::saveGame(const StoryEngine& engine, int slot, const std::string& title) {
    if (slot < 1 || slot > MAX_SLOTS) {
        throw std::invalid_argument("Save slot must be between 1 and " + std::to_string(MAX_SLOTS));
    }

    try {
        const GameState& state = engine.getGameState();
        std::string json = serializeGameState(state);

        // Add title and current node to the JSON
        size_t insertPos = json.rfind('}');
        std::string currentNode = engine.getCurrentNodeId();
        std::string titleStr = title.empty() ? currentNode : title;

        std::string metadata = ",\n  \"title\": \"" + titleStr + "\",\n  \"currentNode\": \"" + currentNode + "\"";
        json.insert(insertPos, metadata);

        fs::path savePath = getSlotPath(slot);
        std::ofstream file(savePath);
        if (!file) {
            throw std::runtime_error("Failed to open save file for writing: " + savePath.string());
        }

        file << json;
        file.close();

        std::cout << "✓ Game saved to slot " << slot << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error saving game: " << e.what() << "\n";
        throw;
    }
}

bool SaveManager::loadGame(StoryEngine& engine, int slot) {
    if (slot < 1 || slot > MAX_SLOTS) {
        std::cerr << "Save slot must be between 1 and " << MAX_SLOTS << "\n";
        return false;
    }

    try {
        fs::path savePath = getSlotPath(slot);
        if (!fs::exists(savePath)) {
            std::cerr << "Save file not found: " << savePath << "\n";
            return false;
        }

        std::ifstream file(savePath);
        if (!file) {
            std::cerr << "Failed to open save file: " << savePath << "\n";
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string json = buffer.str();
        file.close();

        // Extract current node and jump to it
        size_t pos = json.find("\"currentNode\": \"");
        if (pos != std::string::npos) {
            pos += 16;
            size_t endPos = json.find('"', pos);
            std::string nodeId = json.substr(pos, endPos - pos);
            engine.jumpToNode(nodeId);
        }

        // Deserialize game state
        GameState& state = engine.getMutableGameState();
        if (!deserializeGameState(state, json)) {
            return false;
        }

        std::cout << "✓ Game loaded from slot " << slot << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading game: " << e.what() << "\n";
        return false;
    }
}

std::vector<SaveFile> SaveManager::listSaves() const {
    std::vector<SaveFile> saves;

    for (int slot = 1; slot <= MAX_SLOTS; ++slot) {
        saves.push_back(getSaveInfo(slot));
    }

    return saves;
}

SaveFile SaveManager::getSaveInfo(int slot) const {
    SaveFile info{slot, "", "", 0.0, "", false};

    fs::path savePath = getSlotPath(slot);
    if (!fs::exists(savePath)) {
        return info;
    }

    info.exists = true;

    try {
        std::ifstream file(savePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string json = buffer.str();
        file.close();

        // Extract title
        size_t pos = json.find("\"title\": \"");
        if (pos != std::string::npos) {
            pos += 10;
            size_t endPos = json.find('"', pos);
            info.title = json.substr(pos, endPos - pos);
        }

        // Extract timestamp
        pos = json.find("\"timestamp\": \"");
        if (pos != std::string::npos) {
            pos += 14;
            size_t endPos = json.find('"', pos);
            info.timestamp = json.substr(pos, endPos - pos);
        }

        // Extract playtime
        pos = json.find("\"playtimeSecs\": ");
        if (pos != std::string::npos) {
            pos += 16;
            size_t endPos = json.find(',', pos);
            info.playtimeSecs = std::stod(json.substr(pos, endPos - pos));
        }

        // Extract current node
        pos = json.find("\"currentNode\": \"");
        if (pos != std::string::npos) {
            pos += 16;
            size_t endPos = json.find('"', pos);
            info.currentNode = json.substr(pos, endPos - pos);
        }
    } catch (...) {
        // If parsing fails, still return that the file exists
    }

    return info;
}

bool SaveManager::deleteSave(int slot) const {
    if (slot < 1 || slot > MAX_SLOTS) {
        return false;
    }

    try {
        fs::path savePath = getSlotPath(slot);
        if (fs::exists(savePath)) {
            fs::remove(savePath);
            std::cout << "✓ Save slot " << slot << " deleted\n";
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting save: " << e.what() << "\n";
        return false;
    }
}

bool SaveManager::autosave(const StoryEngine& engine) {
    try {
        fs::path autosavePath = savesDir / AUTOSAVE_FILE;
        const GameState& state = engine.getGameState();
        std::string json = serializeGameState(state);

        // Add autosave metadata
        size_t insertPos = json.rfind('}');
        std::string metadata = ",\n  \"title\": \"Autosave\",\n  \"currentNode\": \"" + engine.getCurrentNodeId() + "\"";
        json.insert(insertPos, metadata);

        std::ofstream file(autosavePath);
        if (!file) {
            return false;
        }

        file << json;
        file.close();

        return true;
    } catch (...) {
        return false;
    }
}
