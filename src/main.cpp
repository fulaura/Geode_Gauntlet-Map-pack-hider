#include <Geode/Geode.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/binding/BoomScrollLayer.hpp>
#include <Geode/binding/GauntletNode.hpp>
#include <Geode/binding/GJMapPack.hpp>

using namespace geode::prelude;

static bool isCompletedPack(GJMapPack* pack) {
    if (!pack) return false;
    return pack->hasCompletedMapPack() || 
           GameStatsManager::sharedState()->hasCompletedMapPack(pack->m_packID);
}

class $modify(MyGauntletSelectLayer, GauntletSelectLayer) {
    void setupGauntlets() {
        GauntletSelectLayer::setupGauntlets();

        if (!Mod::get()->getSettingValue<bool>("hide-completed")) return;
        if (!m_scrollLayer || !m_scrollLayer->m_pages) return;

        std::vector<GauntletNode*> activeNodes;
        CCPoint posLeft = CCPointZero;
        CCPoint posRight = CCPointZero;
        bool foundLeft = false;
        bool foundRight = false;

        auto pages = m_scrollLayer->m_pages;
        size_t originalPageCount = pages->count();

        for (size_t p = 0; p < originalPageCount; ++p) {
            auto page = typeinfo_cast<CCLayer*>(pages->objectAtIndex(p));
            if (!page) continue;

            auto children = page->getChildren();
            if (!children) continue;

            std::vector<cocos2d::CCNode*> childrenList;
            for (auto childObj : CCArrayExt<cocos2d::CCNode*>(children)) {
                childrenList.push_back(childObj);
            }

            for (auto childNode : childrenList) {
                GauntletNode* gNode = typeinfo_cast<GauntletNode*>(childNode);
                if (!gNode) {
                    if (auto childChildren = childNode->getChildren()) {
                        for (auto sub : CCArrayExt<cocos2d::CCNode*>(childChildren)) {
                            if (auto g = typeinfo_cast<GauntletNode*>(sub)) {
                                gNode = g;
                                break;
                            }
                        }
                    }
                }

                if (gNode && gNode->m_gauntlet) {
                    if (isCompletedPack(gNode->m_gauntlet)) {
                        childNode->removeFromParentAndCleanup(true);
                    } else {
                        if (!foundLeft) {
                            posLeft = childNode->getPosition();
                            foundLeft = true;
                        } else if (!foundRight && (childNode->getPosition().x != posLeft.x)) {
                            posRight = childNode->getPosition();
                            foundRight = true;
                        }
                        activeNodes.push_back(gNode);
                    }
                }
            }
        }

        if (activeNodes.empty()) {
            pages->removeAllObjects();
            m_scrollLayer->updatePages();
            m_scrollLayer->updateDots(0);
            return;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        if (!foundRight) {
            posRight = CCPoint(winSize.width / 2.0f + 125.0f, posLeft.y);
            if (!foundLeft) {
                posLeft = CCPoint(winSize.width / 2.0f - 125.0f, winSize.height / 2.0f);
            }
        }

        size_t neededPages = (activeNodes.size() + 1) / 2;

        while (pages->count() < neededPages) {
            auto newPage = CCLayer::create();
            m_scrollLayer->addPage(newPage);
        }

        while (pages->count() > neededPages) {
            size_t idx = pages->count() - 1;
            auto pageToRemove = typeinfo_cast<CCLayer*>(pages->objectAtIndex(idx));
            if (pageToRemove) {
                pageToRemove->removeFromParentAndCleanup(true);
            }
            pages->removeObjectAtIndex(idx);
        }

        for (size_t i = 0; i < activeNodes.size(); ++i) {
            size_t targetPageIndex = i / 2;
            auto targetPage = typeinfo_cast<CCLayer*>(pages->objectAtIndex(targetPageIndex));
            if (!targetPage) continue;

            auto node = activeNodes[i];
            cocos2d::CCNode* topNode = node;
            if (node->getParent() && node->getParent() != targetPage) {
                topNode = node->getParent();
            }

            if (topNode->getParent() != targetPage) {
                topNode->retain();
                topNode->removeFromParentAndCleanup(false);
                targetPage->addChild(topNode);
                topNode->release();
            }

            if (i % 2 == 0) {
                if (i == activeNodes.size() - 1) {
                    topNode->setPosition(CCPoint(winSize.width / 2.0f, posLeft.y));
                } else {
                    topNode->setPosition(posLeft);
                }
            } else {
                topNode->setPosition(posRight);
            }
        }

        m_scrollLayer->updatePages();
        m_scrollLayer->updateDots(0);
        m_scrollLayer->moveToPage(0);
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
