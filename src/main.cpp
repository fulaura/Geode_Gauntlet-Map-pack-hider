#include <Geode/Geode.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJMapPack.hpp>

using namespace geode::prelude;

static bool isCompletedGauntlet(GJMapPack* pack, GameStatsManager* gsm) {
    if (!pack) return false;
    return pack->hasCompletedMapPack() || (gsm && gsm->hasCompletedMapPack(pack->m_packID));
}

static void filterGauntletDictionary(cocos2d::CCDictionary* dict) {
    if (!dict) return;

    auto gsm = GameStatsManager::sharedState();
    std::vector<std::string> strKeys;
    std::vector<int> intKeys;

    for (auto [key, obj] : CCDictionaryExt<std::string, CCObject*>(dict)) {
        if (auto pack = typeinfo_cast<GJMapPack*>(obj)) {
            if (isCompletedGauntlet(pack, gsm)) {
                strKeys.push_back(key);
            }
        }
    }

    for (auto [key, obj] : CCDictionaryExt<int, CCObject*>(dict)) {
        if (auto pack = typeinfo_cast<GJMapPack*>(obj)) {
            if (isCompletedGauntlet(pack, gsm)) {
                intKeys.push_back(key);
            }
        }
    }

    for (const auto& k : strKeys) {
        dict->removeObjectForKey(k);
    }
    for (int k : intKeys) {
        dict->removeObjectForKey(k);
    }
}

class $modify(MyGauntletSelectLayer, GauntletSelectLayer) {
    bool init(int unused) {
        if (Mod::get()->getSettingValue<bool>("hide-completed-gauntlets")) {
            if (auto glm = GameLevelManager::sharedState()) {
                filterGauntletDictionary(glm->m_savedGauntlets);
            }
        }
        if (!GauntletSelectLayer::init(unused)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        menu->setID("hide-completed-menu"_spr);
        this->addChild(menu, 100);

        auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        if (offSpr && onSpr) {
            offSpr->setScale(0.8f);
            onSpr->setScale(0.8f);

            auto toggler = CCMenuItemToggler::create(offSpr, onSpr, this, menu_selector(MyGauntletSelectLayer::onToggleHideCompleted));
            bool isHiding = Mod::get()->getSettingValue<bool>("hide-completed-gauntlets");
            toggler->toggle(isHiding);
            toggler->setPosition({winSize.width - 30.0f, winSize.height - 25.0f});
            toggler->setID("hide-completed-toggler"_spr);
            menu->addChild(toggler);

            auto label = CCLabelBMFont::create("Hide", "bigFont.fnt");
            label->setScale(0.35f);
            label->setPosition({winSize.width - 65.0f, winSize.height - 25.0f});
            label->setID("hide-completed-label"_spr);
            this->addChild(label, 100);
        }

        return true;
    }

    void setupGauntlets() {
        if (Mod::get()->getSettingValue<bool>("hide-completed-gauntlets")) {
            if (m_gauntlets) {
                filterGauntletDictionary(m_gauntlets);
            }
        }
        GauntletSelectLayer::setupGauntlets();
    }

    void onToggleHideCompleted(CCObject* sender) {
        bool current = Mod::get()->getSettingValue<bool>("hide-completed-gauntlets");
        Mod::get()->setSettingValue("hide-completed-gauntlets", !current);
        CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.4f, GauntletSelectLayer::scene(0)));
    }
};
