#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace geode::prelude;

static std::optional<std::filesystem::path> findModResource(std::string_view fileName) {
    if (!geode::getMod()) {
        return std::nullopt;
    }

    auto resourcesDir = geode::getMod()->getResourcesDir();
    std::filesystem::path candidates[] = {
        resourcesDir / fileName,
        resourcesDir.parent_path() / fileName,
        resourcesDir.parent_path().parent_path() / fileName,
    };

    for (auto const& path : candidates) {
        if (!path.empty() && std::filesystem::exists(path)) {
            return path;
        }
    }
    return std::nullopt;
}

struct ShowcaseVideoLink {
    std::string url;
    std::string name;
};

static std::unordered_map<int, ShowcaseVideoLink> g_showcaseVideoLinks;
static bool g_showcaseVideoLinksLoaded = false;

static void loadShowcaseVideoLinks() {
    if (g_showcaseVideoLinksLoaded) {
        return;
    }

    if (!geode::getMod()) {
        return;
    }

    auto path = findModResource("showcase-video-links.txt");
    if (!path) {
        log::warn("ShowcaseMod", "Could not find showcase-video-links.txt in mod resources");
        return;
    }

    std::ifstream file(*path);
    if (!file.is_open()) {
        log::warn("ShowcaseMod", "Failed to open resource file: {}", path->generic_string());
        return;
    }

    g_showcaseVideoLinksLoaded = true;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        int id;
        std::string url;
        if (!(iss >> id) || !(iss >> url)) {
            continue;
        }

        std::string name;
        if (std::getline(iss, name) && !name.empty()) {
            if (name.front() == ' ') {
                name.erase(0, 1);
            }
        }

        g_showcaseVideoLinks.emplace(id, ShowcaseVideoLink{std::move(url), std::move(name)});
    }
}

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
                button = CCMenuItemSpriteExtra::create(icon, this, menu_selector(MyLevelInfoLayer::onButtonClick));
            } else {
                auto label = CCLabelBMFont::create("Showcase", "goldFont.fnt");
                if (label) {
                    label->setScale(0.6f);
                    button = CCMenuItemLabel::create(label, this, menu_selector(MyLevelInfoLayer::onButtonClick));
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

        if (geode::getMod()) {
            auto logoPath = findModResource("mod-icon.png");
            if (!logoPath) {
                log::warn("ShowcaseMod", "Logo not found in mod resources");
            } else {
                auto logoPathStr = logoPath->generic_string();
                auto texture = CCTextureCache::sharedTextureCache()->addImage(logoPathStr.c_str(), false);
                if (!texture) {
                    log::warn("ShowcaseMod", "Failed to load logo texture: {}", logoPathStr);
                } else {
                    auto logo = CCSprite::create();
                    if (logo && logo->initWithTexture(texture)) {
                        logo->setScale(0.4f);
                        auto winSize = CCDirector::sharedDirector()->getWinSize();
                        logo->setPosition({
                            logo->getScaledContentSize().width / 2.f + 20.f,
                            winSize.height - logo->getScaledContentSize().height / 2.f - 20.f
                        });
                        logo->setZOrder(100);
                        logo->setID("mod-logo");
                        this->addChild(logo);
                    }
                }
            }
        }

        return true;
    }

    void onButtonClick(CCObject* sender) {
        if (!this->m_level) {
            FLAlertLayer::create("Showcase", "No level is loaded.", "OK")->show();
            return;
        }

        loadShowcaseVideoLinks();
        int levelID = static_cast<int>(this->m_level->m_levelID);
        auto it = g_showcaseVideoLinks.find(levelID);
        if (it != g_showcaseVideoLinks.end()) {
            geode::utils::web::openLinkInBrowser(it->second.url);
        } else {
            FLAlertLayer::create("Showcase", "No showcase video is attached to this level.", "OK")->show();
        }
    }
};
