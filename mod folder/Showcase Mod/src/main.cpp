#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/file.hpp>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace geode::prelude;

struct ShowcaseVideoLink {
    std::string url;
    std::string name;
};

static std::unordered_map<int, ShowcaseVideoLink> g_showcaseVideoLinks;
static bool g_showcaseVideoLinksLoaded = false;

static void loadShowcaseVideoLinks() {
    if (g_showcaseVideoLinksLoaded) return;
    g_showcaseVideoLinksLoaded = true;

    auto path = geode::Mod::get()->getResourcesDir() / "showcase-video-links.txt";
    auto contentRes = geode::utils::file::readString(path);

        if (!contentRes) return;

        std::istringstream stream(*contentRes);
        std::string line;

        while (std::getline(stream, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);

            int id;
            std::string url;

            if (!(iss >> id >> url)) continue;

            std::string name;
            std::getline(iss, name);
            if (!name.empty() && name.front() == ' ')
                name.erase(0, 1);

            g_showcaseVideoLinks[id] = { url, name };
        }
    };

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
public:
 bool init(GJGameLevel* level, bool challenge) {
    if (!LevelInfoLayer::init(level, challenge)) {
        return false;
    }

    auto sideMenu = this->getChildByID("right-side-menu");
    if (!sideMenu) {
        sideMenu = this->getChildByID("side-menu");
    }
    if (!sideMenu) {
        sideMenu = this->getChildByID("top-right-menu");
    }

    if (sideMenu) {
        auto icon = CCSprite::createWithSpriteFrameName("gj_ytIcon_001.png");
        cocos2d::CCMenuItem* button = nullptr;

        if (icon) {
            button = CCMenuItemSpriteExtra::create(
                icon,
                this,
                menu_selector(MyLevelInfoLayer::onButtonClick)
            );
        } else {
            auto label = CCLabelBMFont::create("Showcase", "goldFont.fnt");
            if (label) {
                label->setScale(0.6f);
                button = CCMenuItemLabel::create(
                    label,
                    this,
                    menu_selector(MyLevelInfoLayer::onButtonClick)
                );
            }
        }

        if (button) {
            button->setID("showcase-button");
            sideMenu->addChild(button);
            sideMenu->updateLayout();
        } else {
            log::warn("ShowcaseMod", "Failed to create showcase button for LevelInfoLayer");
        }
    } else {
        log::warn("ShowcaseMod", "Could not find a button container in LevelInfoLayer");
    }

    return true;
}

    void onButtonClick(CCObject* sender) {
        if (!this->m_level) {
            FLAlertLayer::create("Showcase", "No level is loaded.", "OK")->show();
            return;
        }

        int levelID = static_cast<int>(this->m_level->m_levelID);
     auto it = g_showcaseVideoLinks.find(levelID);
        if (it != g_showcaseVideoLinks.end()) {
            geode::utils::web::openLinkInBrowser(it->second.url);
        } else {
            FLAlertLayer::create(
                "Showcase",
                "No showcase video is attached to this level.",
                "OK"
            )->show();
        }
    }
};
