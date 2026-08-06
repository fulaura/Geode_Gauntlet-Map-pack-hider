#include <Geode/Geode.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/CustomListView.hpp>
#include <Geode/modify/MapPackCell.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJMapPack.hpp>
#include <Geode/binding/GJSearchObject.hpp>

using namespace geode::prelude;

static bool isCompletedPack(CCObject* obj) {
    if (!obj) return false;
    if (auto pack = typeinfo_cast<GJMapPack*>(obj)) {
        if (pack->hasCompletedMapPack()) return true;
        if (auto gsm = GameStatsManager::sharedState()) {
            if (gsm->hasCompletedMapPack(pack->m_packID)) return true;
            if (gsm->m_completedMappacks) {
                auto keyStr = std::to_string(pack->m_packID);
                if (gsm->m_completedMappacks->objectForKey(keyStr)) return true;
                if (gsm->m_completedMappacks->objectForKey(pack->m_packID)) return true;
            }
        }
    }
    if (auto dict = typeinfo_cast<CCDictionary*>(obj)) {
        if (auto idObj = dict->objectForKey("1")) {
            int packID = 0;
            if (auto strObj = typeinfo_cast<CCString*>(idObj)) {
                packID = std::atoi(strObj->getCString());
            }
            if (packID > 0) {
                if (auto gsm = GameStatsManager::sharedState()) {
                    if (gsm->hasCompletedMapPack(packID)) return true;
                    if (gsm->m_completedMappacks) {
                        auto keyStr = std::to_string(packID);
                        if (gsm->m_completedMappacks->objectForKey(keyStr)) return true;
                        if (gsm->m_completedMappacks->objectForKey(packID)) return true;
                    }
                }
            }
        }
    }
    return false;
}

static void filterDictionary(cocos2d::CCDictionary* dict) {
    if (!dict) return;

    std::vector<std::string> strKeys;
    std::vector<int> intKeys;

    for (auto [key, obj] : CCDictionaryExt<std::string, CCObject*>(dict)) {
        if (isCompletedPack(obj)) {
            strKeys.push_back(key);
        }
    }

    for (auto [key, obj] : CCDictionaryExt<int, CCObject*>(dict)) {
        if (isCompletedPack(obj)) {
            intKeys.push_back(key);
        }
    }

    for (const auto& k : strKeys) {
        dict->removeObjectForKey(k);
    }
    for (int k : intKeys) {
        dict->removeObjectForKey(k);
    }
}

static void filterArray(cocos2d::CCArray* arr) {
    if (!arr) return;
    auto toRemove = CCArray::create();
    for (auto obj : CCArrayExt<CCObject*>(arr)) {
        if (isCompletedPack(obj)) {
            toRemove->addObject(obj);
        }
    }
    for (auto obj : CCArrayExt<CCObject*>(toRemove)) {
        arr->removeObject(obj);
    }
}

class $modify(MyCustomListView, CustomListView) {
    bool init(cocos2d::CCArray* entries, TableViewCellDelegate* delegate, float height, float width, int page, BoomListType type, float y) {
        if (type == BoomListType::MapPack && Mod::get()->getSettingValue<bool>("hide-completed-mappacks") && entries) {
            filterArray(entries);
        }
        return CustomListView::init(entries, delegate, height, width, page, type, y);
    }
};

class $modify(MyMapPackCell, MapPackCell) {
    void loadFromMapPack(GJMapPack* pack) {
        MapPackCell::loadFromMapPack(pack);
        if (Mod::get()->getSettingValue<bool>("hide-completed-mappacks")) {
            if (isCompletedPack(pack)) {
                this->setVisible(false);
            }
        }
    }
};

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
        if (object && object->m_searchType == SearchType::MapPack) {
            if (Mod::get()->getSettingValue<bool>("hide-completed-mappacks")) {
                if (auto glm = GameLevelManager::sharedState()) {
                    filterDictionary(glm->m_savedPacks);
                }
            }
        }

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
                toggler->setPosition({winSize.width - 30.0f, 25.0f});
                toggler->setID("hide-completed-toggler"_spr);
                menu->addChild(toggler);

                auto label = CCLabelBMFont::create("Hide", "bigFont.fnt");
                label->setScale(0.35f);
                label->setPosition({winSize.width - 65.0f, 25.0f});
                label->setID("hide-completed-label"_spr);
                this->addChild(label, 100);
            }
        }

        return true;
    }

    void setupLevelBrowser(cocos2d::CCArray* items) {
        if (m_searchObject && m_searchObject->m_searchType == SearchType::MapPack && Mod::get()->getSettingValue<bool>("hide-completed-mappacks")) {
            filterArray(items);
            filterArray(m_levels);
        }
        LevelBrowserLayer::setupLevelBrowser(items);
    }

    void loadLevelsFinished(cocos2d::CCArray* levels, char const* key, int type) {
        if (m_searchObject && m_searchObject->m_searchType == SearchType::MapPack && Mod::get()->getSettingValue<bool>("hide-completed-mappacks")) {
            filterArray(levels);
            filterArray(m_levels);
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
