#include "../include/script_parser.h"
#include <toml++/toml.h>
#include <filesystem>
#include <stdexcept>
#include <iostream>

namespace fs = std::filesystem;

Script ScriptParser::loadFromFile(const std::string& path, bool validateAssets) {
    toml::table doc = toml::parse_file(path);
    Script script;

    for (auto&& [k, v] : doc) {
        const std::string id = std::string(k.str());
        const toml::table* node = v.as_table();
        if (!node) continue;

        const std::string ctx = "node [" + id + "]";
        StoryNode n;

        n.speaker = node->get("speaker") ? node->get("speaker")->value_or<std::string>("Narrator") : "Narrator";
        n.bgFile = node->get("bg") ? node->get("bg")->value_or<std::string>("") : "";
        n.charFile = node->get("char") ? node->get("char")->value_or<std::string>("") : "";

        parseDialogueAndDescription(*node, ctx, n.dialogue, n.descriptionPages);
        n.pages = n.descriptionPages;

        if (auto* charsArr = (*node)["chars"].as_array()) {
            for (auto&& el : *charsArr) {
                const toml::table* ct = el.as_table();
                if (!ct) throw std::runtime_error("Invalid chars entry (not a table) in " + ctx);

                StoryNode::CharacterDef cd;
                cd.file = ct->get("file") ? ct->get("file")->value_or<std::string>("") : "";
                cd.pos = ct->get("pos") ? ct->get("pos")->value_or<std::string>("center") : "center";
                cd.heightFrac = ct->get("height") ? static_cast<float>(ct->get("height")->value_or<double>(DEFAULT_CHARACTER_HEIGHT_FRAC)) : DEFAULT_CHARACTER_HEIGHT_FRAC;
                cd.xOffset = ct->get("x") ? static_cast<float>(ct->get("x")->value_or<double>(0.0)) : 0.f;
                cd.yOffset = ct->get("y") ? static_cast<float>(ct->get("y")->value_or<double>(0.0)) : 0.f;
                cd.z = ct->get("z") ? static_cast<int>(ct->get("z")->value_or<int64_t>(0)) : 0;
                cd.alpha = ct->get("alpha") ? static_cast<float>(ct->get("alpha")->value_or<double>(1.0)) : 1.f;
                cd.heightFrac = std::clamp(cd.heightFrac, 0.05f, 1.50f);
                cd.alpha = std::clamp(cd.alpha, 0.f, 1.f);

                if (cd.file.empty()) throw std::runtime_error("chars entry missing 'file' in " + ctx);
                n.chars.push_back(std::move(cd));
            }
        }

        n.next = node->get("next") ? node->get("next")->value_or<std::string>("") : "";
        n.bgm = node->get("bgm") ? node->get("bgm")->value_or<std::string>("") : "";
        n.bgmLoop = node->get("bgm_loop") ? node->get("bgm_loop")->value_or<bool>(true) : true;
        n.bgmVolume = node->get("bgm_volume") ? static_cast<float>(node->get("bgm_volume")->value_or<double>(DEFAULT_BGM_VOLUME)) : DEFAULT_BGM_VOLUME;
        n.bgmVolume = std::clamp(n.bgmVolume, 0.f, 100.f);

        if (auto* choicesArr = (*node)["choices"].as_array()) {
            if (choicesArr->size() > MAX_CHOICES) {
                throw std::runtime_error(ctx + " has " + std::to_string(choicesArr->size()) + " choices (max " + std::to_string(MAX_CHOICES) + ")");
            }
            for (size_t i = 0; i < choicesArr->size(); i++) {
                const toml::table* c = (*choicesArr)[i].as_table();
                if (!c) throw std::runtime_error("Invalid choice (not a table) in " + ctx);

                ChoiceDef cd;
                cd.key = c->get("key") ? c->get("key")->value_or<std::string>("") : "";
                cd.text = c->get("text") ? c->get("text")->value_or<std::string>("") : "";
                cd.next = c->get("next") ? c->get("next")->value_or<std::string>("") : "";
                cd.sfx = c->get("sfx") ? c->get("sfx")->value_or<std::string>("") : "";

                if (auto* condsArr = c->get("conditions") ? c->get("conditions")->as_array() : nullptr) {
                    for (auto&& cond : *condsArr) {
                        auto s = cond.value<std::string>();
                        if (s) cd.conditions.push_back(*s);
                    }
                }

                if (auto* setVarsTable = c->get("set_vars") ? c->get("set_vars")->as_table() : nullptr) {
                    for (auto&& [varName, varVal] : *setVarsTable) {
                        if (auto v = varVal.value<double>()) {
                            cd.setVars[std::string(varName.str())] = *v;
                        } else if (auto v = varVal.value<int64_t>()) {
                            cd.setVars[std::string(varName.str())] = static_cast<double>(*v);
                        }
                    }
                }

                if (auto* flagsArr = c->get("set_flags") ? c->get("set_flags")->as_array() : nullptr) {
                    for (auto&& flag : *flagsArr) {
                        auto s = flag.value<std::string>();
                        if (s) cd.setFlags.push_back(*s);
                    }
                }

                if (cd.key.empty()) cd.key = (i == 0 ? "A" : (i == 1 ? "B" : "C"));
                validateChoiceKey(cd.key, ctx);

                if (cd.text.empty()) throw std::runtime_error("Choice " + cd.key + " missing 'text' in " + ctx);
                if (cd.next.empty()) throw std::runtime_error("Choice " + cd.key + " missing 'next' in " + ctx);

                n.choices.push_back(std::move(cd));
            }
        }

        if (validateAssets) {
            validateFileExists(n.bgFile, ctx);
            validateFileExists(n.charFile, ctx);
            for (const auto& c : n.chars) validateFileExists(c.file, ctx + " (chars)");
            validateFileExists(n.bgm, ctx + " (bgm)");
            for (const auto& ch : n.choices) {
                validateFileExists(ch.sfx, ctx + " (choice " + ch.key + " sfx)");
            }
        }

        auto [it, inserted] = script.nodes.emplace(id, std::move(n));
        if (!inserted) throw std::runtime_error("Duplicate node id: " + id);
    }

    if (script.nodes.find("start") == script.nodes.end()) {
        throw std::runtime_error("Script missing required node [start]");
    }

    static const std::unordered_map<std::string, bool> kConfigKeys = {
        {"game", {true}}, {"window", {true}}, {"fonts", {true}},
        {"title_screen", {true}}, {"meta", {true}}};

    for (const auto& [id, node] : script.nodes) {
        if (kConfigKeys.count(id)) continue;

        for (const auto& c : node.choices) {
            if (script.nodes.find(c.next) == script.nodes.end()) {
                throw std::runtime_error("Node [" + id + "] choice " + c.key +
                                       " points to missing node [" + c.next + "]");
            }
        }

        if (!node.next.empty() && script.nodes.find(node.next) == script.nodes.end()) {
            throw std::runtime_error("Node [" + id + "] next=\"" + node.next +
                                   "\" points to missing node [" + node.next + "]");
        }

        if (node.choices.empty() && node.next.empty()) {
            std::cerr << "Warning: node [" << id << "] has no choices and no 'next' — player cannot advance past it\n";
        }
    }

    return script;
}

void ScriptParser::validateChoiceKey(const std::string& key, const std::string& context) {
    if (key == "A" || key == "B" || key == "C") return;
    throw std::runtime_error("Invalid choice key '" + key + "' in " + context + " (must be A/B/C)");
}

void ScriptParser::validateFileExists(const std::string& path, const std::string& context) {
    if (path.empty()) return;
    if (!fs::exists(path)) {
        throw std::runtime_error("Missing asset file: '" + path + "' referenced by " + context);
    }
}

void ScriptParser::parseDialogueAndDescription(
    const toml::table& t,
    const std::string& context,
    std::string& outDialogue,
    std::vector<std::string>& outDescription) {
    (void)context;

    if (auto v = t["dialogue"].value<std::string>()) {
        outDialogue = *v;
    } else if (auto* arr = t["dialogue"].as_array()) {
        for (auto&& el : *arr) {
            auto s = el.value<std::string>();
            if (!s) throw std::runtime_error("Non-string in dialogue array");
            outDescription.push_back(*s);
        }
        outDialogue = "";
    }

    if (auto* descArr = t["description"].as_array()) {
        for (auto&& el : *descArr) {
            auto s = el.value<std::string>();
            if (!s) throw std::runtime_error("Non-string in description array");
            outDescription.push_back(*s);
        }
    } else if (auto v = t["description"].value<std::string>()) {
        outDescription.push_back(*v);
    }

    if (outDescription.empty()) {
        outDescription.push_back("");
    }
}

std::vector<std::string> ScriptParser::readPagesLegacy(const toml::table& t, const std::string& context) {
    (void)context;
    if (auto v = t["dialogue"].value<std::string>()) {
        return {*v};
    }
    if (auto* arr = t["dialogue"].as_array()) {
        std::vector<std::string> pages;
        pages.reserve(arr->size());
        for (auto&& el : *arr) {
            auto s = el.value<std::string>();
            if (!s) throw std::runtime_error("Non-string in dialogue array");
            pages.push_back(*s);
        }
        if (pages.empty()) pages.push_back("");
        return pages;
    }
    return {""};
}
