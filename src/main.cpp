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
        if (Mod::get()->getSettingValue<bool>("hide-completed")) {
            if (auto glm = GameLevelManager::sharedState()) {
                filterDictionary(glm->m_savedGauntlets);
            }
        }
        return GauntletSelectLayer::init(unused);
    }

    void setupGauntlets() {
        if (Mod::get()->getSettingValue<bool>("hide-completed")) {
            if (m_gauntlets) {
                filterDictionary(m_gauntlets);
            }
            if (auto glm = GameLevelManager::sharedState()) {
                filterDictionary(glm->m_savedGauntlets);
            }
        }
        GauntletSelectLayer::setupGauntlets();
    }
};

class $modify(MyLevelBrowserLayer, LevelBrowserLayer) {
    void setupLevelBrowser(cocos2d::CCArray* items) {
        if (Mod::get()->getSettingValue<bool>("hide-completed") && items) {
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
        if (Mod::get()->getSettingValue<bool>("hide-completed") && levels) {
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
};
