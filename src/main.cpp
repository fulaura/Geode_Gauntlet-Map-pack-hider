#include <Geode/Geode.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

using namespace geode::prelude;

static bool isCompletedPack(GJMapPack* pack) {
    if (!pack) return false;
    return pack->hasCompletedMapPack() || 
           GameStatsManager::sharedState()->hasCompletedMapPack(pack->m_packID);
}

class $modify(MyGauntletSelectLayer, GauntletSelectLayer) {
    void loadLevelsFinished(cocos2d::CCArray* gauntlets, char const* key, int type) {
        if (Mod::get()->getSettingValue<bool>("hide-completed") && gauntlets) {
            auto toRemove = CCArray::create();
            for (auto obj : CCArrayExt<GJMapPack*>(gauntlets)) {
                if (isCompletedPack(obj)) {
                    toRemove->addObject(obj);
                }
            }
            for (auto obj : CCArrayExt<GJMapPack*>(toRemove)) {
                gauntlets->removeObject(obj);
            }
        }
        GauntletSelectLayer::loadLevelsFinished(gauntlets, key, type);
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
};
