#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace geode::prelude;

struct ShowcaseVideoLink {
    std::string url;
    std::string name;
};

static std::unordered_map<int, ShowcaseVideoLink> g_showcaseVideoLinks;
static bool g_loaded = false;

static void loadShowcaseVideoLinks() {
    if (g_loaded) return;
    if (!geode::getMod()) return;

    auto path = geode::getMod()->getResourcesDir() / "showcase-video-links.txt";

    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        log::warn("ShowcaseMod", "Video link file not found");
        return;
    }

    std::ifstream file(geode::utils::string::pathToString(path));
    if (!file.is_open()) {
        log::warn("ShowcaseMod", "Could not open video link file");
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);

        int id = 0;
        std::string url;

        if (!(ss >> id >> url)) continue;

        std::string name;
        std::getline(ss, name);

        if (!name.empty() && name.front() == ' ')
            name.erase(0, 1);

        g_showcaseVideoLinks.emplace(id, ShowcaseVideoLink{ url, name });
    }

    g_loaded = true;
}

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
public:
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto menu = this->getChildByID("right-side-menu");
        if (!menu) menu = this->getChildByID("side-menu");
        if (!menu) menu = this->getChildByID("top-right-menu");

        if (menu) {
            auto icon = CCSprite::createWithSpriteFrameName("gj_ytIcon_001.png");

            CCMenuItem* btn = nullptr;

            if (icon) {
                btn = CCMenuItemSpriteExtra::create(
                    icon, this,
                    menu_selector(MyLevelInfoLayer::onButtonClick)
                );
            } else {
                auto label = CCLabelBMFont::create("Showcase", "goldFont.fnt");
                if (label) {
                    label->setScale(0.6f);
                    btn = CCMenuItemLabel::create(
                        label, this,
                        menu_selector(MyLevelInfoLayer::onButtonClick)
                    );
                }
            }

            if (btn) {
                btn->setID("showcase-button");
                menu->addChild(btn);
                menu->updateLayout();
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

        geode::utils::web::openLinkInBrowser(it->second.url);
    }
};