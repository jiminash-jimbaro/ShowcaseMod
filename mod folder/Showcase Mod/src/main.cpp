#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <fmt/core.h>
#include <unordered_map>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace geode::prelude;

struct ShowcaseVideoLink {
    std::string url;
    std::string name;
};

static std::unordered_map<int, ShowcaseVideoLink> g_showcaseVideoLinks;
static bool g_loaded = false;

static void loadShowcaseVideoLinks() {
    if (g_loaded) return;

    // Get path to the showcase video links file
    auto path = geode::getMod()->getResourcesDir() / "showcase-video-links.txt";
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        log::warn("ShowcaseMod", fmt::format("Video link file not found"));
        return;
    }

    // Open the file and process each line
    std::ifstream file(geode::utils::string::pathToString(path));
    if (!file.is_open()) {
        log::warn("ShowcaseMod", fmt::format("Could not open video link file"));
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // Skip empty or comment lines

        // Parse each line into ID, URL, and Name
        std::istringstream ss(line);
        int id = 0;
        std::string url;
        if (!(ss >> id >> url)) continue; // If parsing fails, skip the line
        std::string name;
        std::getline(ss, name);
        if (!name.empty() && name.front() == ' ') name.erase(0, 1); // Remove leading space

        g_showcaseVideoLinks.emplace(id, ShowcaseVideoLink{ url, name });
    }
    g_loaded = true;
}

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
public:
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        loadShowcaseVideoLinks();

        int levelID = (int)level->m_levelID;
        auto it = g_showcaseVideoLinks.find(levelID);
        if (it != g_showcaseVideoLinks.end()) { // Only create button if video exists
            auto menu = this->getChildByID("left-side-menu"); // Menu where button will go
            if (menu) {
                auto icon = CCSprite::createWithSpriteFrameName("gj_ytIcon_001.png"); // Icon for button
                CCMenuItem* btn = nullptr; // Create button with icon if available
                if (icon) {
                    btn = CCMenuItemSpriteExtra::create(icon, this, menu_selector(MyLevelInfoLayer::onButtonClick));
                } else { // Fallback to label if no icon found
                    auto label = CCLabelBMFont::create("Showcase", "goldFont.fnt");
                    if (label) {
                        label->setScale(0.6f);
                        btn = CCMenuItemLabel::create(label, this, menu_selector(MyLevelInfoLayer::onButtonClick));
                    }
                }

                // Add button to menu if created
                if (btn) {
                    btn->setID("showcase-button"_spr);
                    menu->addChild(btn);
                    menu->updateLayout();
                }
            }
        }
        return true;
    }

    void onButtonClick(CCObject*) {
        if (!m_level) {
            FLAlertLayer::create("Showcase", "No level loaded", "OK")->show();
            return;
        }

        loadShowcaseVideoLinks();
        int id = (int)m_level->m_levelID;
        auto it = g_showcaseVideoLinks.find(id);
        if (it == g_showcaseVideoLinks.end()) {
            FLAlertLayer::create("Showcase", "No video for this level", "OK")->show();
            return;
        }

        // Open the video in the browser
        geode::utils::web::openLinkInBrowser(it->second.url);
    }
};