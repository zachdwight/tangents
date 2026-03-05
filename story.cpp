// story.cpp — SFML 3.x VN demo with TOML script loading (max 3 choices)
//
// Build:
//clang++ -std=c++20 -O2 story.cpp  -I/usr/local/include -L/usr/local/lib  -Wl,-rpath,/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system  -o AnimeGame
//
// Run:
//   ./AnimeGame
//
// Script format (top-level tables):
//   [start]
//   speaker = "???"
//   bg = "images/environments/Diner.png"
//   char = "images/characters/Annie.png"
//   dialogue = ["page 1", "page 2"]
//   [[start.choices]] key="A" text="..." next="A"
//   [[start.choices]] key="B" text="..." next="B"
//   [[start.choices]] key="C" text="..." next="C"   # optional, max 3
//
// Notes:
// - dialogue can be array-of-strings (preferred) OR a single string.
// - choices is optional; if present, must be 0..3; key defaults to A/B/C if omitted.
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <toml++/toml.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

// -------------------- Data model --------------------

struct ChoiceDef {
    std::string key;   // "A"/"B"/"C"
    std::string text;  // display text
    std::string next;  // node id
    std::string sfx;   // OPTIONAL: sound played when this choice is picked
};

struct StoryNode {
    std::string speaker;
    std::vector<std::string> pages; // click-to-advance pages
    std::string bgFile;
    std::string charFile;

    // NEW: multiple characters support. If chars is non-empty, it overrides charFile.
    struct CharacterDef {
        std::string file;           // texture path
        std::string pos = "center"; // left|center|right OR numeric 0..1 as string
        float heightFrac = 0.80f;   // height relative to screen
        float xOffset = 0.f;        // pixels
        float yOffset = 0.f;        // pixels (positive moves down)
        int z = 0;                  // draw order (low -> high)
        float alpha = 1.f;          // 0..1
    };
    std::vector<CharacterDef> chars;
    std::vector<ChoiceDef> choices; // 0..3

    // OPTIONAL: auto-advance target when there are no choices
    std::string next;

    // OPTIONAL: background music for this node (streamed)
    // Convention: empty string => stop BGM
    std::string bgm;
    bool bgmLoop = true;
    float bgmVolume = 70.f; // 0..100
};

struct Script {
    std::unordered_map<std::string, StoryNode> nodes;
};

// -------------------- Helpers: wrap & scaling --------------------

static std::vector<std::string> splitWords(const std::string& s) {
    std::vector<std::string> words;
    std::string cur;
    for (char c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!cur.empty()) {
                words.push_back(cur);
                cur.clear();
            }
        } else cur.push_back(c);
    }
    if (!cur.empty()) words.push_back(cur);
    return words;
}

static void setWrappedString(sf::Text& text, const std::string& str, float maxWidthPx) {
    std::string out;
    std::string paragraph;

    for (size_t i = 0; i <= str.size(); i++) {
        char c = (i < str.size()) ? str[i] : '\n';
        if (c == '\n') {
            const auto words = splitWords(paragraph);
            std::string line;

            for (size_t wi = 0; wi < words.size(); wi++) {
                std::string candidate = line.empty() ? words[wi] : (line + " " + words[wi]);
                text.setString(candidate);

                if (text.getLocalBounds().size.x > maxWidthPx && !line.empty()) {
                    out += line + "\n";
                    line = words[wi];
                } else {
                    line = candidate;
                }
            }

            out += line;
            out += "\n";
            paragraph.clear();
        } else {
            paragraph.push_back(c);
        }
    }

    if (!out.empty() && out.back() == '\n') out.pop_back();
    text.setString(out);
}

static void scaleSpriteCover(sf::Sprite& spr, const sf::Texture& tex, float w, float h) {
    auto ts = tex.getSize();
    if (!ts.x || !ts.y) return;

    float sx = w / static_cast<float>(ts.x);
    float sy = h / static_cast<float>(ts.y);
    float s = std::max(sx, sy); // cover screen

    spr.setScale({s, s});

    float drawnW = static_cast<float>(ts.x) * s;
    float drawnH = static_cast<float>(ts.y) * s;
    spr.setPosition({(w - drawnW) / 2.f, (h - drawnH) / 2.f});
}

static void scaleCharacterBottom(sf::Sprite& spr, const sf::Texture& tex, float w, float h, float heightFrac = 0.80f) {
    auto ts = tex.getSize();
    if (!ts.x || !ts.y) return;

    float targetHeight = h * heightFrac;
    float s = targetHeight / static_cast<float>(ts.y);

    spr.setScale({s, s});
    sf::FloatRect b = spr.getGlobalBounds();
    spr.setPosition({(w - b.size.x) / 2.f, h - b.size.y});
}

// Like scaleCharacterBottom, but supports anchors and offsets.
static void placeCharacterBottom(sf::Sprite& spr,
                                const sf::Texture& tex,
                                float w,
                                float h,
                                float heightFrac,
                                float anchor01,
                                float xOffsetPx,
                                float yOffsetPx) {
    auto ts = tex.getSize();
    if (!ts.x || !ts.y) return;

    float targetHeight = h * heightFrac;
    float s = targetHeight / static_cast<float>(ts.y);
    spr.setScale({s, s});

    // Bounds depend on scale.
    sf::FloatRect b = spr.getGlobalBounds();

    // anchor01 is 0..1 representing desired center position as fraction of screen width.
    float xCenter = std::clamp(anchor01, 0.f, 1.f) * w;
    float x = xCenter - (b.size.x / 2.f) + xOffsetPx;
    float y = (h - b.size.y) + yOffsetPx;
    spr.setPosition({x, y});
}

// -------------------- Texture cache --------------------

struct TextureCache {
    std::map<std::string, sf::Texture> textures;

    sf::Texture& get(const std::string& path) {
        auto it = textures.find(path);
        if (it != textures.end()) return it->second;

        sf::Texture tex;
        if (!tex.loadFromFile(path)) {
            throw std::runtime_error("Failed to load texture: " + path);
        }
        auto ins = textures.emplace(path, std::move(tex));
        return ins.first->second;
    }
};

// -------------------- Sound cache (SFX) --------------------

struct SoundCache {
    std::map<std::string, sf::SoundBuffer> buffers;

    sf::SoundBuffer& get(const std::string& path) {
        auto it = buffers.find(path);
        if (it != buffers.end()) return it->second;

        sf::SoundBuffer buf;
        if (!buf.loadFromFile(path)) {
            throw std::runtime_error("Failed to load sound: " + path);
        }
        auto ins = buffers.emplace(path, std::move(buf));
        return ins.first->second;
    }
};

// -------------------- TOML loading + validation --------------------

static void validate_choice_key(const std::string& key, const std::string& ctx) {
    if (key == "A" || key == "B" || key == "C") return;
    throw std::runtime_error("Invalid choice key '" + key + "' in " + ctx + " (must be A/B/C)");
}

static void validate_file_exists(const std::string& path, const std::string& ctx) {
    if (path.empty()) return;
    if (!fs::exists(path)) {
        throw std::runtime_error("Missing asset file: '" + path + "' referenced by " + ctx);
    }
}

static std::vector<std::string> read_pages(const toml::table& t, const std::string& ctx) {
    (void)ctx;
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

static Script load_script_toml(const std::string& path, bool validateAssets = true) {
    toml::table doc = toml::parse_file(path);

    Script script;

    for (auto&& [k, v] : doc) {
        const std::string id = std::string(k.str());
        const toml::table* node = v.as_table();
        if (!node) continue;

        const std::string ctx = "node [" + id + "]";

        StoryNode n;
        n.speaker = node->get("speaker") ? node->get("speaker")->value_or<std::string>("Narrator") : "Narrator";
        n.bgFile  = node->get("bg")      ? node->get("bg")->value_or<std::string>("") : "";
        n.charFile= node->get("char")    ? node->get("char")->value_or<std::string>("") : "";
        n.pages   = read_pages(*node, ctx);

        // NEW: multi-character array. Example:
        // [[start.chars]] file="..." pos="left" height=0.85 x=0 y=0 z=0 alpha=1.0
        // [[start.chars]] file="..." pos="right" z=1
        if (auto* charsArr = (*node)["chars"].as_array()) {
            for (auto&& el : *charsArr) {
                const toml::table* ct = el.as_table();
                if (!ct) throw std::runtime_error("Invalid chars entry (not a table) in " + ctx);

                StoryNode::CharacterDef cd;
                cd.file = ct->get("file") ? ct->get("file")->value_or<std::string>("") : "";
                cd.pos  = ct->get("pos")  ? ct->get("pos")->value_or<std::string>("center") : "center";
                cd.heightFrac = ct->get("height") ? static_cast<float>(ct->get("height")->value_or<double>(0.80)) : 0.80f;
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

        // Auto-advance target (used when the node has no choices)
        n.next = node->get("next") ? node->get("next")->value_or<std::string>("") : "";

        // NEW: BGM fields
        n.bgm = node->get("bgm") ? node->get("bgm")->value_or<std::string>("") : "";
        n.bgmLoop = node->get("bgm_loop") ? node->get("bgm_loop")->value_or<bool>(true) : true;
        n.bgmVolume = node->get("bgm_volume") ? static_cast<float>(node->get("bgm_volume")->value_or<double>(70.0)) : 70.f;
        n.bgmVolume = std::clamp(n.bgmVolume, 0.f, 100.f);

        if (auto* choicesArr = (*node)["choices"].as_array()) {
            if (choicesArr->size() > 3) {
                throw std::runtime_error(ctx + " has " + std::to_string(choicesArr->size()) + " choices (max 3)");
            }
            for (size_t i = 0; i < choicesArr->size(); i++) {
                const toml::table* c = (*choicesArr)[i].as_table();
                if (!c) throw std::runtime_error("Invalid choice (not a table) in " + ctx);

                ChoiceDef cd;
                cd.key  = c->get("key")  ? c->get("key")->value_or<std::string>("") : "";
                cd.text = c->get("text") ? c->get("text")->value_or<std::string>("") : "";
                cd.next = c->get("next") ? c->get("next")->value_or<std::string>("") : "";
                cd.sfx  = c->get("sfx")  ? c->get("sfx")->value_or<std::string>("") : ""; // NEW

                if (cd.key.empty()) cd.key = (i == 0 ? "A" : (i == 1 ? "B" : "C"));
                validate_choice_key(cd.key, ctx);

                if (cd.text.empty()) throw std::runtime_error("Choice " + cd.key + " missing 'text' in " + ctx);
                if (cd.next.empty()) throw std::runtime_error("Choice " + cd.key + " missing 'next' in " + ctx);

                n.choices.push_back(std::move(cd));
            }
        }

        if (validateAssets) {
            validate_file_exists(n.bgFile, ctx);
            validate_file_exists(n.charFile, ctx);
            for (const auto& c : n.chars) validate_file_exists(c.file, ctx + " (chars)");
            // Optional: validate audio files if specified
            validate_file_exists(n.bgm, ctx + " (bgm)");
            for (const auto& ch : n.choices) {
                validate_file_exists(ch.sfx, ctx + " (choice " + ch.key + " sfx)");
            }
        }

        auto [it, inserted] = script.nodes.emplace(id, std::move(n));
        if (!inserted) throw std::runtime_error("Duplicate node id: " + id);
    }

    if (script.nodes.find("start") == script.nodes.end()) {
        throw std::runtime_error("Script missing required node [start]");
    }

    // Known config-only top-level tables — not story nodes.
    static const std::unordered_map<std::string, bool> kConfigKeys = {
        {"game",{true}}, {"window",{true}}, {"fonts",{true}},
        {"title_screen",{true}}, {"meta",{true}}
    };

    for (const auto& [id, node] : script.nodes) {
        if (kConfigKeys.count(id)) continue; // skip engine-config sections

        // Validate choice targets exist.
        for (const auto& c : node.choices) {
            if (script.nodes.find(c.next) == script.nodes.end()) {
                throw std::runtime_error("Node [" + id + "] choice " + c.key +
                                         " points to missing node [" + c.next + "]");
            }
        }

        // Validate next target exists.
        if (!node.next.empty() && script.nodes.find(node.next) == script.nodes.end()) {
            throw std::runtime_error("Node [" + id + "] next=\"" + node.next +
                                     "\" points to missing node [" + node.next + "]");
        }

        // Warn about dead-end nodes (no choices and no next).
        if (node.choices.empty() && node.next.empty()) {
            std::cout << "Warning: node [" + id + "] has no choices and no 'next' — player cannot advance past it\n";
        }
    }

    return script;
}

// -------------------- Engine UI types --------------------

struct BacklogEntry {
    std::string speaker;
    std::string text;
};

enum class ChoiceKey { None, A, B, C };


// -------------------- Game config / Title screen --------------------

struct GameConfig {
    std::string title = "Default Title";
    std::string version = "0.0.0";
    std::string company = "Default Company";
    unsigned width = 1920;
    unsigned height = 1080;

    std::string uiFont = "fonts/Delius-Regular.ttf";

    // Title screen assets (all optional)
    std::string titleBg = "";
    std::string titleLogo = "";
    std::string startButtonText = "Start";
    std::string hintText = "Click Start or press Enter";

    // Optional title BGM (streamed)
    std::string titleBgm = "";
    bool titleBgmLoop = true;
    float titleBgmVolume = 55.f;
};

static GameConfig load_game_config_toml(const std::string& scriptPath) {
    GameConfig cfg;
    try {
        auto tbl = toml::parse_file(scriptPath);

        if (auto v = tbl["game"]["title"].value<std::string>()) cfg.title = *v;
        if (auto v = tbl["game"]["version"].value<std::string>()) cfg.version = *v;
        if (auto v = tbl["game"]["company"].value<std::string>()) cfg.company = *v;

        if (auto v = tbl["window"]["width"].value<int64_t>()) cfg.width = static_cast<unsigned>(*v);
        if (auto v = tbl["window"]["height"].value<int64_t>()) cfg.height = static_cast<unsigned>(*v);

        if (auto v = tbl["fonts"]["ui"].value<std::string>()) cfg.uiFont = *v;

        if (auto v = tbl["title_screen"]["background"].value<std::string>()) cfg.titleBg = *v;
        if (auto v = tbl["title_screen"]["logo"].value<std::string>()) cfg.titleLogo = *v;
        if (auto v = tbl["title_screen"]["start_button_text"].value<std::string>()) cfg.startButtonText = *v;
        if (auto v = tbl["title_screen"]["hint_text"].value<std::string>()) cfg.hintText = *v;

        if (auto v = tbl["title_screen"]["bgm"].value<std::string>()) cfg.titleBgm = *v;
        if (auto v = tbl["title_screen"]["bgm_loop"].value<bool>()) cfg.titleBgmLoop = *v;
        if (auto v = tbl["title_screen"]["bgm_volume"].value<int64_t>()) cfg.titleBgmVolume = static_cast<float>(*v);

    } catch (const std::exception& e) {
        std::cout << "Warning: failed to load game config from '" << scriptPath
                  << "': " << e.what() << " — using defaults\n";
    }
    return cfg;
}

struct UiButton {
    sf::RectangleShape box;
    sf::Text label;
    bool hovered = false;

    explicit UiButton(const sf::Font& font) : label(font) {}

    void setSize(sf::Vector2f s) { box.setSize(s); }

    void setText(const std::string& t, unsigned sizePx) {
        label.setString(t);
        label.setCharacterSize(sizePx);
    }

    void setPosition(float x, float y) {
        box.setPosition({x, y});
        auto b = label.getLocalBounds();
        label.setPosition({
            x + (box.getSize().x - b.size.x) * 0.5f - b.position.x,
            y + (box.getSize().y - b.size.y) * 0.5f - b.position.y
        });
    }

    bool contains(sf::Vector2f p) const { return box.getGlobalBounds().contains(p); }

    void draw(sf::RenderWindow& w) const { w.draw(box); w.draw(label); }
};

static void setCuteButtonStyle(UiButton& btn, bool hovered) {
    btn.hovered = hovered;
    btn.box.setFillColor(hovered ? sf::Color(255, 210, 235) : sf::Color(255, 190, 225));
    btn.box.setOutlineThickness(2.f);
    btn.box.setOutlineColor(hovered ? sf::Color(255, 255, 255) : sf::Color(240, 240, 240));
    btn.label.setFillColor(sf::Color(60, 40, 55));
}

static bool run_title_screen(sf::RenderWindow& window, const GameConfig& cfg, const sf::Font& font) {
    // Optional background
    std::optional<sf::Texture> bgTex;
    std::optional<sf::Sprite> bg;
    if (!cfg.titleBg.empty() && fs::exists(cfg.titleBg)) {
        bgTex.emplace();
        if (bgTex->loadFromFile(cfg.titleBg)) {
            bg.emplace(*bgTex);
            scaleSpriteCover(*bg, *bgTex, static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y));
        } else {
            bg.reset();
            bgTex.reset();
        }
    }

    // Optional logo
    std::optional<sf::Texture> logoTex;
    std::optional<sf::Sprite> logo;
    if (!cfg.titleLogo.empty() && fs::exists(cfg.titleLogo)) {
        logoTex.emplace();
        if (logoTex->loadFromFile(cfg.titleLogo)) {
            logo.emplace(*logoTex);
            auto s = logoTex->getSize();
            if (s.x && s.y) {
                float targetW = window.getSize().x * 0.55f;
                float scale = targetW / static_cast<float>(s.x);
                logo->setScale({scale, scale});
                logo->setPosition({
                    window.getSize().x * 0.5f - (static_cast<float>(s.x) * scale) * 0.5f,
                    window.getSize().y * 0.10f
                });
            }
        } else {
            logo.reset();
            logoTex.reset();
        }
    }

    // Optional title BGM
    sf::Music titleMusic;
    if (!cfg.titleBgm.empty() && fs::exists(cfg.titleBgm)) {
        if (titleMusic.openFromFile(cfg.titleBgm)) {
            titleMusic.setLooping(cfg.titleBgmLoop);
            titleMusic.setVolume(std::clamp(cfg.titleBgmVolume, 0.f, 100.f));
            titleMusic.play();
        }
    }

    sf::Text titleText(font);
    titleText.setString(cfg.title);
    titleText.setCharacterSize(58);
    titleText.setStyle(sf::Text::Bold);
    titleText.setFillColor(sf::Color(255, 250, 240));
    titleText.setOutlineThickness(3.f);
    titleText.setOutlineColor(sf::Color(10, 10, 10, 160));
    titleText.setPosition({60.f, 44.f});

    sf::Text metaText(font);
    metaText.setCharacterSize(20);
    metaText.setFillColor(sf::Color(255, 245, 240));
    metaText.setOutlineThickness(2.f);
    metaText.setOutlineColor(sf::Color(0, 0, 0, 140));
    {
        std::string meta;
        if (!cfg.company.empty()) meta += cfg.company + "   ";
        meta += "v" + cfg.version;
        metaText.setString(meta);
    }
    metaText.setPosition({62.f, 112.f});

    sf::Text hint(font);
    hint.setCharacterSize(18);
    hint.setFillColor(sf::Color(255, 255, 255, 210));
    hint.setOutlineThickness(2.f);
    hint.setOutlineColor(sf::Color(0, 0, 0, 160));
    hint.setString(cfg.hintText);

    UiButton start(font);
    start.setSize({280.f, 72.f});
    start.setText(cfg.startButtonText, 30);
    float bx = window.getSize().x * 0.5f - start.box.getSize().x * 0.5f;
    float by = window.getSize().y * 0.70f;
    start.setPosition(bx, by);
    setCuteButtonStyle(start, false);

    // Center hint under button
    {
        auto hb = hint.getLocalBounds();
        hint.setPosition({
            window.getSize().x * 0.5f - (hb.size.x * 0.5f) - hb.position.x,
            by + 92.f
        });
    }

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) return false;

            if (auto* r = ev->getIf<sf::Event::Resized>()) {
                sf::View view(sf::FloatRect({0.f, 0.f}, {static_cast<float>(r->size.x), static_cast<float>(r->size.y)}));
                window.setView(view);

                // Refit bg if present
                if (bg && bgTex) {
                    scaleSpriteCover(*bg, *bgTex, static_cast<float>(r->size.x), static_cast<float>(r->size.y));
                }

                // Reposition button + hint
                float nbx = window.getSize().x * 0.5f - start.box.getSize().x * 0.5f;
                float nby = window.getSize().y * 0.70f;
                start.setPosition(nbx, nby);

                auto hb = hint.getLocalBounds();
                hint.setPosition({
                    window.getSize().x * 0.5f - (hb.size.x * 0.5f) - hb.position.x,
                    nby + 92.f
                });
            }

            if (auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Enter || kp->code == sf::Keyboard::Key::Space) return true;
                if (kp->code == sf::Keyboard::Key::Escape) return false;
            }

            if (auto* mm = ev->getIf<sf::Event::MouseMoved>()) {
                auto mp = window.mapPixelToCoords({mm->position.x, mm->position.y});
                setCuteButtonStyle(start, start.contains(mp));
            }

            if (auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    auto mp = window.mapPixelToCoords({mb->position.x, mb->position.y});
                    if (start.contains(mp)) return true;
                }
            }
        }

        window.clear(sf::Color(26, 18, 32));
        if (bg) window.draw(*bg);

        // If there is a logo, draw it; otherwise title text carries the scene.
        if (logo) window.draw(*logo);

        window.draw(titleText);
        window.draw(metaText);
        start.draw(window);
        window.draw(hint);

        window.display();
    }

    return false;
}

// -------------------- Main --------------------

int main(int argc, char** argv) {
    const std::string scriptPath = (argc >= 2) ? argv[1] : "script.toml";

    
    const GameConfig cfg = load_game_config_toml(scriptPath);
Script script;
    try {
        script = load_script_toml(scriptPath, /*validateAssets=*/true);
    } catch (const std::exception& e) {
        std::cout << "Script load failed: " << e.what() << "\n";
        return -1;
    }

    sf::RenderWindow window(sf::VideoMode({cfg.width, cfg.height}), cfg.title);
    window.setFramerateLimit(60);

    // Title screen first
    // (Uses the same UI font as the game UI)

    sf::Font font;
    if (!font.openFromFile(cfg.uiFont)) {
        std::cout << "Error: Could not open UI font: " << cfg.uiFont << "\n";
        return -1;
    }

    if (!run_title_screen(window, cfg, font)) return 0;

    TextureCache cache;
    std::optional<sf::Sprite> bgSprite;
    std::string currentBgFile;

    // NEW: multiple characters
    std::vector<sf::Sprite> charSprites;
    std::vector<StoryNode::CharacterDef> currentCharDefs;

    // NEW: Audio objects
    sf::Music bgmPlayer;
    std::string currentBgmFile;
    SoundCache sfxCache;
    std::vector<sf::Sound> sfxVoices;
    sfxVoices.reserve(8);

    auto playSfx = [&](const std::string& file, float vol = 85.f) {
        if (file.empty()) return;
        try {
            // clean finished voices
            sfxVoices.erase(
                std::remove_if(sfxVoices.begin(), sfxVoices.end(),
                               [](const sf::Sound& s) { return s.getStatus() == sf::Sound::Status::Stopped; }),
                sfxVoices.end()
            );

            // SFML 3: sf::Sound requires a buffer at construction time (no default ctor).
            const sf::SoundBuffer& buf = sfxCache.get(file);
            sfxVoices.emplace_back(buf);
            sf::Sound& snd = sfxVoices.back();
            snd.setVolume(std::clamp(vol, 0.f, 100.f));
            snd.play();
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    };

    // UI shapes (IMPORTANT: no {} init for SFML 3 explicit ctors)
    sf::RectangleShape panel;
    std::array<sf::RectangleShape, 3> btnBox;
    sf::RectangleShape backlogPanel;

    // UI text
    sf::Text nameText(font), dialogueText(font), hintText(font);
    std::array<sf::Text, 3> btnText = {sf::Text(font), sf::Text(font), sf::Text(font)};
    sf::Text backlogTitle(font), backlogText(font);

    // Styling
    nameText.setCharacterSize(22);
    nameText.setStyle(sf::Text::Bold);
    nameText.setFillColor(sf::Color(255, 250, 240));
    nameText.setOutlineThickness(2.f);
    nameText.setOutlineColor(sf::Color(10, 10, 10, 140));

    dialogueText.setCharacterSize(22);
    dialogueText.setFillColor(sf::Color(255, 250, 240));
    dialogueText.setOutlineThickness(2.f);
    dialogueText.setOutlineColor(sf::Color(10, 10, 10, 140));

    hintText.setCharacterSize(16);
    hintText.setFillColor(sf::Color(255, 255, 255, 200));
    hintText.setOutlineThickness(2.f);
    hintText.setOutlineColor(sf::Color(0, 0, 0, 160));

    for (auto& t : btnText) {
        t.setCharacterSize(20);
        t.setFillColor(sf::Color(230, 255, 255));
        t.setOutlineThickness(2.f);
        t.setOutlineColor(sf::Color(10, 10, 10, 140));
    }

    backlogTitle.setCharacterSize(22);
    backlogTitle.setStyle(sf::Text::Bold);
    backlogTitle.setFillColor(sf::Color(255, 250, 240));
    backlogTitle.setOutlineThickness(2.f);
    backlogTitle.setOutlineColor(sf::Color(0, 0, 0, 160));

    backlogText.setCharacterSize(18);
    backlogText.setFillColor(sf::Color(255, 250, 240));
    backlogText.setOutlineThickness(2.f);
    backlogText.setOutlineColor(sf::Color(0, 0, 0, 160));

    panel.setFillColor(sf::Color(0, 0, 0, 150));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(255, 255, 255, 60));

    for (auto& b : btnBox) {
        b.setFillColor(sf::Color(0, 0, 0, 130));
        b.setOutlineThickness(2.f);
        b.setOutlineColor(sf::Color(0, 255, 255, 120));
    }

    backlogPanel.setFillColor(sf::Color(0, 0, 0, 200));
    backlogPanel.setOutlineThickness(2.f);
    backlogPanel.setOutlineColor(sf::Color(255, 255, 255, 70));

    // Backlog state
    std::vector<BacklogEntry> backlog;
    bool backlogOpen = false;
    int backlogScroll = 0;

    // Typewriter state
    bool typewriterOn = true;
    float charsPerSecond = 55.f;
    sf::Clock typeClock;

    // Dialogue/page state
    std::string currentNodeId = "start";
    std::string lastLoadedNodeId = "__INIT__";
    std::vector<std::string> pages;
    size_t pageIndex = 0;
    std::string wrappedPageText;
    size_t revealedChars = 0;

    // Choice visibility/presence
    std::array<bool, 3> choicePresent = {false, false, false};

    auto ws = [&]() -> sf::Vector2f {
        auto s = window.getSize();
        return {static_cast<float>(s.x), static_cast<float>(s.y)};
    };

    auto layoutUI = [&]() {
        auto v = ws();
        float w = v.x;
        float h = v.y;

        float panelH = std::max(200.f, h * 0.32f);
        float pad = std::max(18.f, h * 0.02f);

        panel.setSize({w - 2.f * pad, panelH});
        panel.setPosition({pad, h - panelH - pad});

        nameText.setPosition({panel.getPosition().x + pad, panel.getPosition().y + pad * 0.6f});
        dialogueText.setPosition({panel.getPosition().x + pad, nameText.getPosition().y + 34.f});

        float btnY = panel.getPosition().y + panelH - pad - 44.f;
        float gap = pad * 0.6f;
        float btnW = (panel.getSize().x - 2.f * pad - 2.f * gap) / 3.f;
        float btnH = 44.f;

        for (int i = 0; i < 3; i++) {
            btnBox[i].setSize({btnW, btnH});
            btnBox[i].setPosition({panel.getPosition().x + pad + i * (btnW + gap), btnY});
            btnText[i].setPosition({btnBox[i].getPosition().x + 12.f, btnBox[i].getPosition().y + 8.f});
        }

        hintText.setPosition({panel.getPosition().x + pad, panel.getPosition().y + panelH - pad - 88.f});

        float bw = w * 0.86f;
        float bh = h * 0.80f;
        backlogPanel.setSize({bw, bh});
        backlogPanel.setPosition({(w - bw) / 2.f, (h - bh) / 2.f});

        backlogTitle.setString("Backlog ✎  (Esc/L to close, wheel to scroll)");
        backlogTitle.setPosition({backlogPanel.getPosition().x + 18.f, backlogPanel.getPosition().y + 14.f});

        backlogText.setPosition({backlogPanel.getPosition().x + 18.f, backlogPanel.getPosition().y + 56.f});
    };

    auto parseAnchor01 = [&](const std::string& pos) -> float {
        std::string p;
        p.reserve(pos.size());
        for (char c : pos) p.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

        if (p == "left") return 0.25f;
        if (p == "center" || p == "middle") return 0.50f;
        if (p == "right") return 0.75f;

        // Numeric 0..1
        try {
            float v = std::stof(pos);
            return std::clamp(v, 0.f, 1.f);
        } catch (...) {
            return 0.50f;
        }
    };

    auto rebuildCharacters = [&]() {
        charSprites.clear();
        if (currentCharDefs.empty()) return;

        auto v = ws();
        charSprites.reserve(currentCharDefs.size());

        // Ensure deterministic draw order.
        std::vector<size_t> order(currentCharDefs.size());
        for (size_t i = 0; i < order.size(); i++) order[i] = i;
        std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) {
            return currentCharDefs[a].z < currentCharDefs[b].z;
        });

        for (size_t idx : order) {
            const auto& cd = currentCharDefs[idx];
            try {
                auto& tex = cache.get(cd.file);
                sf::Sprite spr(tex);

                float anchor01 = parseAnchor01(cd.pos);
                placeCharacterBottom(spr, tex, v.x, v.y, cd.heightFrac, anchor01, cd.xOffset, cd.yOffset);

                // Apply alpha to sprite color.
                sf::Color c = spr.getColor();
                c.a = static_cast<uint8_t>(std::clamp(cd.alpha, 0.f, 1.f) * 255.f);
                spr.setColor(c);

                charSprites.push_back(std::move(spr));
            } catch (const std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
    };

    auto rebuildBacklogText = [&]() {
        std::string big;
        for (const auto& e : backlog) big += e.speaker + ": " + e.text + "\n\n";

        std::vector<std::string> lines;
        std::string cur;
        for (char c : big) {
            if (c == '\n') { lines.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        lines.push_back(cur);

        int total = static_cast<int>(lines.size());
        int visible = 28;
        backlogScroll = std::clamp(backlogScroll, 0, std::max(0, total - visible));

        int start = std::max(0, total - visible - backlogScroll);
        int end = std::min(total, start + visible);

        std::string windowStr;
        for (int i = start; i < end; i++) windowStr += lines[i] + "\n";

        float maxW = backlogPanel.getSize().x - 36.f;
        backlogText.setString("");
        setWrappedString(backlogText, windowStr, maxW);
    };

    auto preparePage = [&]() {
        if (pages.empty()) pages = {""};
        if (pageIndex >= pages.size()) pageIndex = pages.size() - 1;

        float pad = std::max(18.f, static_cast<float>(window.getSize().y) * 0.02f);
        float maxTextWidth = panel.getSize().x - 2.f * pad;

        dialogueText.setString("");
        setWrappedString(dialogueText, pages[pageIndex], maxTextWidth);
        wrappedPageText = dialogueText.getString();

        revealedChars = 0;
        typeClock.restart();
        dialogueText.setString("");
    };

    auto pageFullyShown = [&]() -> bool {
        return (!typewriterOn) || (revealedChars >= wrappedPageText.size());
    };

    auto nodeFinishedText = [&]() -> bool {
        if (pages.empty()) return true;
        bool onLastPage = (pageIndex + 1 >= pages.size());
        return onLastPage && pageFullyShown();
    };

    auto revealAll = [&]() {
        revealedChars = wrappedPageText.size();
        dialogueText.setString(wrappedPageText);
    };

    auto advance = [&]() {
        if (typewriterOn && revealedChars < wrappedPageText.size()) {
            revealAll();
            return;
        }
        if (pageIndex + 1 < pages.size()) {
            pageIndex++;
            preparePage();
            return;
        }
        // On the last page with no choices: follow `next` if provided.
        auto it = script.nodes.find(currentNodeId);
        if (it != script.nodes.end() && it->second.choices.empty() && !it->second.next.empty()) {
            currentNodeId = it->second.next;
            loadNode();
        }
    };

    auto canChoose = [&](ChoiceKey ck) -> bool {
        auto it = script.nodes.find(currentNodeId);
        if (it == script.nodes.end()) return false;
        const auto& ch = it->second.choices;
        if (ck == ChoiceKey::A) return ch.size() >= 1;
        if (ck == ChoiceKey::B) return ch.size() >= 2;
        if (ck == ChoiceKey::C) return ch.size() >= 3;
        return false;
    };

    auto applyChoice = [&](ChoiceKey ck) {
        auto it = script.nodes.find(currentNodeId);
        if (it == script.nodes.end()) return;
        const auto& ch = it->second.choices;

        int idx = (ck == ChoiceKey::A ? 0 : (ck == ChoiceKey::B ? 1 : 2));
        if (idx < 0 || idx >= static_cast<int>(ch.size())) return;

        currentNodeId = ch[idx].next;
    };

    std::function<void()> loadNode;
    loadNode = [&]() {
        if (currentNodeId == lastLoadedNodeId) return;

        auto it = script.nodes.find(currentNodeId);
        if (it == script.nodes.end()) {
            nameText.setString("Narrator");
            pages = {"Missing node: " + currentNodeId};
            pageIndex = 0;
            preparePage();

            choicePresent = {false, false, false};
            for (int i = 0; i < 3; i++) btnText[i].setString("");

            lastLoadedNodeId = currentNodeId;
            return;
        }

        const StoryNode& n = it->second;

        // NEW: BGM handling per node
        if (n.bgm.empty()) {
            if (!currentBgmFile.empty()) {
                bgmPlayer.stop();
                currentBgmFile.clear();
            }
        } else if (n.bgm != currentBgmFile) {
            if (bgmPlayer.openFromFile(n.bgm)) {
                bgmPlayer.setLooping(n.bgmLoop);
                bgmPlayer.setVolume(n.bgmVolume);
                bgmPlayer.play();
                currentBgmFile = n.bgm;
            } else {
                std::cout << "Failed to open BGM: " << n.bgm << "\n";
            }
        } else {
            // same track; allow per-node tweaks
            bgmPlayer.setLooping(n.bgmLoop);
            bgmPlayer.setVolume(n.bgmVolume);
        }

        // backlog: join pages
        std::string joined;
        for (size_t i = 0; i < n.pages.size(); i++) {
            joined += n.pages[i];
            if (i + 1 < n.pages.size()) joined += "\n\n";
        }
        backlog.push_back({n.speaker.empty() ? "Narrator" : n.speaker, joined});
        if (backlog.size() > 200) backlog.erase(backlog.begin());
        rebuildBacklogText();

        nameText.setString(n.speaker.empty() ? "Narrator" : n.speaker);

        pages = n.pages;
        pageIndex = 0;
        preparePage();

        // buttons (track which exist)
        choicePresent = {false, false, false};
        for (int i = 0; i < 3; i++) btnText[i].setString("");

        float maxW = btnBox[0].getSize().x - 24.f;
        int count = static_cast<int>(n.choices.size());
        for (int i = 0; i < count && i < 3; i++) {
            choicePresent[i] = true;
            setWrappedString(btnText[i], n.choices[i].key + ": " + n.choices[i].text, maxW);
        }

        currentBgFile = n.bgFile;

        // NEW: character set for this node.
        // If n.chars provided, render all of them; otherwise fall back to legacy n.charFile.
        currentCharDefs.clear();
        if (!n.chars.empty()) {
            currentCharDefs = n.chars;
        } else if (!n.charFile.empty()) {
            StoryNode::CharacterDef cd;
            cd.file = n.charFile;
            cd.pos = "center";
            cd.heightFrac = 0.80f;
            cd.xOffset = 0.f;
            cd.yOffset = 0.f;
            cd.z = 0;
            cd.alpha = 1.f;
            currentCharDefs.push_back(std::move(cd));
        }

        try {
            auto& bgTex = cache.get(currentBgFile);
            if (!bgSprite) bgSprite.emplace(bgTex);
            else bgSprite->setTexture(bgTex, true);
            auto v = ws();
            scaleSpriteCover(*bgSprite, bgTex, v.x, v.y);
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }

        rebuildCharacters();

        lastLoadedNodeId = currentNodeId;
    };

    layoutUI();
    loadNode();

    while (window.isOpen()) {
        ChoiceKey pendingChoice = ChoiceKey::None;
        bool pendingAdvance = false;

        bool showChoicesNow = (!backlogOpen) && nodeFinishedText();

        // tiny polish: dynamic hint
        {
            bool hasChoices = (choicePresent[0] || choicePresent[1] || choicePresent[2]);
            bool midReveal = (!pageFullyShown());
            bool hasNextPage = (pageIndex + 1 < pages.size());

            if (backlogOpen) {
                hintText.setString("Reading log… (Esc/L to close) ✎");
            } else if (midReveal) {
                hintText.setString("Click / Space to skip the typing… ✨");
            } else if (hasNextPage) {
                hintText.setString("Click / Space to continue… ➜");
            } else if (hasChoices) {
                hintText.setString("Pick a choice: A / B / C ✨");
            } else {
                hintText.setString("The End? (Press Esc to quit) 🌙");
            }
        }

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::View v(sf::FloatRect({0.f, 0.f},
                                         {static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)}));
                window.setView(v);

                layoutUI();

                if (bgSprite && !currentBgFile.empty()) {
                    try {
                        auto& bgTex = cache.get(currentBgFile);
                        scaleSpriteCover(*bgSprite, bgTex,
                                         static_cast<float>(resized->size.x),
                                         static_cast<float>(resized->size.y));
                    } catch (...) {}
                }
                // Rebuild all character placements for new window size
                rebuildCharacters();

                preparePage();
                rebuildBacklogText();
            }

            if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (backlogOpen) {
                    backlogScroll += (wheel->delta > 0) ? -3 : 3;
                    rebuildBacklogText();
                }
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    if (backlogOpen) backlogOpen = false;
                    else window.close();
                }

                if (keyPressed->code == sf::Keyboard::Key::L) {
                    backlogOpen = !backlogOpen;
                    rebuildBacklogText();
                }

                if (keyPressed->code == sf::Keyboard::Key::T) {
                    typewriterOn = !typewriterOn;
                    if (!typewriterOn) revealAll();
                }

                if (keyPressed->code == sf::Keyboard::Key::Equal) charsPerSecond = std::min(240.f, charsPerSecond + 10.f);
                if (keyPressed->code == sf::Keyboard::Key::Hyphen) charsPerSecond = std::max(10.f, charsPerSecond - 10.f);

                if (!backlogOpen) {
                    // Only allow choosing when choices are visible
                    if (showChoicesNow) {
                        if (keyPressed->code == sf::Keyboard::Key::A && choicePresent[0]) pendingChoice = ChoiceKey::A;
                        if (keyPressed->code == sf::Keyboard::Key::B && choicePresent[1]) pendingChoice = ChoiceKey::B;
                        if (keyPressed->code == sf::Keyboard::Key::C && choicePresent[2]) pendingChoice = ChoiceKey::C;
                    }

                    if (keyPressed->code == sf::Keyboard::Key::Space ||
                        keyPressed->code == sf::Keyboard::Key::Enter) {
                        pendingAdvance = true;
                    }
                }
            }

            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mp{static_cast<float>(mb->position.x), static_cast<float>(mb->position.y)};

                    if (backlogOpen) {
                        if (!backlogPanel.getGlobalBounds().contains(mp)) backlogOpen = false;
                    } else {
                        if (showChoicesNow) {
                            if (choicePresent[0] && btnBox[0].getGlobalBounds().contains(mp)) pendingChoice = ChoiceKey::A;
                            else if (choicePresent[1] && btnBox[1].getGlobalBounds().contains(mp)) pendingChoice = ChoiceKey::B;
                            else if (choicePresent[2] && btnBox[2].getGlobalBounds().contains(mp)) pendingChoice = ChoiceKey::C;
                            else pendingAdvance = true;
                        } else {
                            pendingAdvance = true;
                        }
                    }
                }
            }
        }

        // Typewriter update
        if (!backlogOpen) {
            if (typewriterOn && revealedChars < wrappedPageText.size()) {
                float dt = typeClock.restart().asSeconds();
                float speed = charsPerSecond;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                    speed *= 3.f;
                }

                size_t add = static_cast<size_t>(speed * dt);
                if (add < 1) add = 1;
                revealedChars = std::min(wrappedPageText.size(), revealedChars + add);
                dialogueText.setString(wrappedPageText.substr(0, revealedChars));
            } else {
                typeClock.restart();
            }
        } else {
            typeClock.restart();
        }

        // Hover/enable styling (only when visible + present)
        auto mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f m{static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)};

        for (int i = 0; i < 3; i++) {
            bool enabled = (i == 0 ? canChoose(ChoiceKey::A) : (i == 1 ? canChoose(ChoiceKey::B) : canChoose(ChoiceKey::C)));
            bool visible = showChoicesNow && choicePresent[i] && enabled;
            bool hover = visible && btnBox[i].getGlobalBounds().contains(m);

            if (!visible) {
                btnBox[i].setOutlineColor(sf::Color(0, 0, 0, 0));
                btnBox[i].setFillColor(sf::Color(0, 0, 0, 0));
                continue;
            }

            btnBox[i].setOutlineColor(hover ? sf::Color(255, 255, 120, 200) : sf::Color(0, 255, 255, 120));
            btnBox[i].setFillColor(hover ? sf::Color(0, 0, 0, 165) : sf::Color(0, 0, 0, 130));
        }

        // Apply choice (NEW: play SFX if defined on that choice)
        if (!backlogOpen && pendingChoice != ChoiceKey::None) {
            bool ok =
                (pendingChoice == ChoiceKey::A && canChoose(ChoiceKey::A) && choicePresent[0]) ||
                (pendingChoice == ChoiceKey::B && canChoose(ChoiceKey::B) && choicePresent[1]) ||
                (pendingChoice == ChoiceKey::C && canChoose(ChoiceKey::C) && choicePresent[2]);

            if (ok) {
                auto it = script.nodes.find(currentNodeId);
                if (it != script.nodes.end()) {
                    const auto& ch = it->second.choices;
                    int idx = (pendingChoice == ChoiceKey::A ? 0 : (pendingChoice == ChoiceKey::B ? 1 : 2));
                    if (idx >= 0 && idx < static_cast<int>(ch.size())) {
                        playSfx(ch[idx].sfx); // ✅ choice click sound
                    }
                }

                applyChoice(pendingChoice);
                loadNode();
            }
        }

        // Apply advance
        if (!backlogOpen && pendingAdvance) {
            advance();
        }

        // Render
        window.clear();

        if (bgSprite) window.draw(*bgSprite);
        for (auto& s : charSprites) window.draw(s);

        window.draw(panel);
        window.draw(nameText);
        window.draw(dialogueText);
        window.draw(hintText);

        if (showChoicesNow) {
            for (int i = 0; i < 3; i++) {
                if (!choicePresent[i]) continue;
                bool enabled = (i == 0 ? canChoose(ChoiceKey::A) : (i == 1 ? canChoose(ChoiceKey::B) : canChoose(ChoiceKey::C)));
                if (!enabled) continue;

                window.draw(btnBox[i]);
                window.draw(btnText[i]);
            }
        }

        if (backlogOpen) {
            window.draw(backlogPanel);
            window.draw(backlogTitle);
            window.draw(backlogText);
        }

        window.display();
    }

    return 0;
}