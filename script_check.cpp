#include <toml++/toml.h>
#include <filesystem>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <vector>
#include <optional>

namespace fs = std::filesystem;

struct ChoiceDef {
    std::string key;   // A/B/C
    std::string text;
    std::string next;
};

struct StoryBeat {
    std::string speaker;
    std::vector<std::string> dialoguePages;
    std::string bgFile;
    std::string charFile;
    std::vector<ChoiceDef> choices; // 0..3
};

struct Script {
    std::unordered_map<std::string, StoryBeat> nodes;
};

// Helpers
static std::string require_string(const toml::table& t, const char* field, const std::string& context) {
    if (auto v = t[field].value<std::string>()) return *v;
    throw std::runtime_error("Missing/invalid string '" + std::string(field) + "' in " + context);
}

static std::optional<std::string> opt_string(const toml::table& t, const char* field) {
    if (auto v = t[field].value<std::string>()) return *v;
    return std::nullopt;
}

static std::vector<std::string> require_string_array(const toml::table& t, const char* field, const std::string& context) {
    const auto* arr = t[field].as_array();
    if (!arr) throw std::runtime_error("Missing/invalid array '" + std::string(field) + "' in " + context);

    std::vector<std::string> out;
    out.reserve(arr->size());
    for (auto&& el : *arr) {
        auto s = el.value<std::string>();
        if (!s) throw std::runtime_error("Non-string element in '" + std::string(field) + "' in " + context);
        out.push_back(*s);
    }
    return out;
}

static void validate_file_exists(const std::string& path, const std::string& context) {
    if (path.empty()) return; // allow empty if you want
    if (!fs::exists(path)) {
        throw std::runtime_error("Missing asset file: '" + path + "' referenced by " + context);
    }
}

static void validate_choice_key(const std::string& key, const std::string& context) {
    if (key == "A" || key == "B" || key == "C") return;
    throw std::runtime_error("Invalid choice key '" + key + "' in " + context + " (must be A/B/C)");
}

static Script load_script_toml(const std::string& tomlPath, bool validateAssets = true) {
    toml::table doc = toml::parse_file(tomlPath);

    Script script;

    // In the example TOML, nodes are top-level tables: [start], [A], etc.
    // So doc is a table of tables.
    for (auto&& [nodeIdKey, nodeVal] : doc) {
        const std::string nodeId = std::string(nodeIdKey.str());

        const toml::table* node = nodeVal.as_table();
        if (!node) continue; // skip non-tables at top-level

        const std::string ctx = "node [" + nodeId + "]";

        StoryBeat beat;
        beat.speaker = opt_string(*node, "speaker").value_or("Narrator");
        beat.bgFile  = opt_string(*node, "bg").value_or("");
        beat.charFile = opt_string(*node, "char").value_or("");

        // dialogue: allow either string OR array-of-strings (array is preferred)
        if (auto s = node->get("dialogue")->value<std::string>()) {
            beat.dialoguePages = {*s};
        } else if (node->contains("dialogue")) {
            beat.dialoguePages = require_string_array(*node, "dialogue", ctx);
        } else {
            beat.dialoguePages = {""};
        }

        // choices: optional
        beat.choices.clear();
        if (auto* choicesArr = (*node)["choices"].as_array()) {
            if (choicesArr->size() > 3) {
                throw std::runtime_error(ctx + " has " + std::to_string(choicesArr->size()) + " choices (max 3)");
            }
            for (size_t i = 0; i < choicesArr->size(); i++) {
                const toml::table* c = (*choicesArr)[i].as_table();
                if (!c) throw std::runtime_error("Invalid choice entry (not a table) in " + ctx);

                ChoiceDef cd;
                cd.key  = c->get("key")  ? c->get("key")->value_or<std::string>("") : "";
                cd.text = c->get("text") ? c->get("text")->value_or<std::string>("") : "";
                cd.next = c->get("next") ? c->get("next")->value_or<std::string>("") : "";

                if (cd.key.empty()) cd.key = (i == 0 ? "A" : (i == 1 ? "B" : "C"));
                validate_choice_key(cd.key, ctx);

                if (cd.text.empty())
                    throw std::runtime_error("Choice '" + cd.key + "' missing 'text' in " + ctx);
                if (cd.next.empty())
                    throw std::runtime_error("Choice '" + cd.key + "' missing 'next' in " + ctx);

                beat.choices.push_back(std::move(cd));
            }
        }

        if (validateAssets) {
            validate_file_exists(beat.bgFile, ctx);
            validate_file_exists(beat.charFile, ctx);
        }

        // Store node
        auto [it, inserted] = script.nodes.emplace(nodeId, std::move(beat));
        if (!inserted) throw std::runtime_error("Duplicate node id: " + nodeId);
    }

    // Cross-reference validation: every choice.next must exist
    for (const auto& [id, beat] : script.nodes) {
        for (const auto& c : beat.choices) {
            if (!script.nodes.contains(c.next)) {
                throw std::runtime_error("Node [" + id + "] choice " + c.key +
                                         " points to missing node [" + c.next + "]");
            }
        }
    }

    // Optional: ensure a start node exists
    if (!script.nodes.contains("start")) {
        throw std::runtime_error("Script missing required node [start]");
    }

    return script;
}
