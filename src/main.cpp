#include <Geode/Geode.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJMapPack.hpp>

using namespace geode::prelude;

static bool isCompletedPack(GJMapPack* pack) {
    if (!pack) return false;
    return pack->hasCompletedMapPack() || 
           GameStatsManager::sharedState()->hasCompletedMapPack(pack->m_packID);
}

static void filterDictionary(cocos2d::CCDictionary* dict) {
    if (!dict) return;

    std::vector<std::string> strKeys;
    std::vector<int> intKeys;

    for (auto [key, obj] : CCDictionaryExt<std::string, CCObject*>(dict)) {
        if (auto pack = typeinfo_cast<GJMapPack*>(obj)) {
            if (isCompletedPack(pack)) {
                strKeys.push_back(key);
            }
        }
    }

    for (auto [key, obj] : CCDictionaryExt<int, CCObject*>(dict)) {
        if (auto pack = typeinfo_cast<GJMapPack*>(obj)) {
            if (isCompletedPack(pack)) {
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
                filterDictionary(glm->m_savedGauntlets);
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
            toggler->setPosition({winSize.width - 30.0f, 30.0f});
            toggler->setID("hide-completed-toggler"_spr);
            menu->addChild(toggler);

            auto label = CCLabelBMFont::create("Hide", "bigFont.fnt");
            label->setScale(0.35f);
            label->setPosition({winSize.width - 65.0f, 30.0f});
            label->setID("hide-completed-label"_spr);
            this->addChild(label, 100);
        }

        return true;
    }

    void setupGauntlets() {
        if (Mod::get()->getSettingValue<bool>("hide-completed-gauntlets")) {
            if (m_gauntlets) {
                filterDictionary(m_gauntlets);
            }
            if (auto glm = GameLevelManager::sharedState()) {
                filterDictionary(glm->m_savedGauntlets);
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

class $modify(MyLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject* object) {
        if (!LevelBrowserLayer::init(object)) return false;

        if (object && object->m_searchType == SearchType::MapPack) {
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

                auto toggler = CCMenuItemToggler::create(offSpr, onSpr, this, menu_selector(MyLevelBrowserLayer::onToggleHideCompleted));
                bool isHiding = Mod::get()->getSettingValue<bool>("hide-completed-mappacks");
                toggler->toggle(isHiding);
                toggler->setPosition({winSize.width - 30.0f, 30.0f});
                toggler->setID("hide-completed-toggler"_spr);
                menu->addChild(toggler);

                auto label = CCLabelBMFont::create("Hide", "bigFont.fnt");
                label->setScale(0.35f);
                label->setPosition({winSize.width - 65.0f, 30.0f});
                label->setID("hide-completed-label"_spr);
                this->addChild(label, 100);
            }
        }

        return true;
    }

    void setupLevelBrowser(cocos2d::CCArray* items) {
        if (Mod::get()->getSettingValue<bool>("hide-completed-mappacks") && items) {
            auto toRemove = CCArray::create();
            for (auto obj : CCArrayExt<CCObject*>(items)) {
                if (auto pack = typeinfo_cast<GJMapPack*>(obj)) {
                    if (isCompletedPack(pack)) {
                        toRemove->addObject(pack);
                    }
                }
            }
            for (auto obj : CCArrayExt<CCObject*>(toRemove)) {
                items->removeObject(obj);
            }
        }
        LevelBrowserLayer::setupLevelBrowser(items);
    }

    void loadLevelsFinished(cocos2d::CCArray* levels, char const* key, int type) {
        if (Mod::get()->getSettingValue<bool>("hide-completed-mappacks") && levels) {
            auto toRemove = CCArray::create();
            for (auto obj : CCArrayExt<CCObject*>(levels)) {
                if (auto pack = typeinfo_cast<GJMapPack*>(obj)) {
                    if (isCompletedPack(pack)) {
                        toRemove->addObject(pack);
                    }
                }
            }
            for (auto obj : CCArrayExt<CCObject*>(toRemove)) {
                levels->removeObject(obj);
            }
        }
        LevelBrowserLayer::loadLevelsFinished(levels, key, type);
    }

    void onToggleHideCompleted(CCObject* sender) {
        bool current = Mod::get()->getSettingValue<bool>("hide-completed-mappacks");
        Mod::get()->setSettingValue("hide-completed-mappacks", !current);
        if (m_searchObject) {
            this->loadPage(m_searchObject);
        }
    }
};
