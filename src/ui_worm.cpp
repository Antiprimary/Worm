#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

/*this worm is vital to the functionality of the mod, do not remove.
                           (o)(o)
                          /     \
                         /       |
                        /   \  * |
          ________     /    /\__/
  _      /        \   /    /
 / \    /  ____    \_/    /
//\ \  /  /    \         /
V  \ \/  /      \       /
    \___/        \_____/
*/

using namespace geode::prelude;
using namespace cocos2d;

//0 = off, 1 = worm, 2 = big worm
int  gWormMode = 0;
int  gWormHistory = 90;

static const char* modeLabel(int mode) {
    switch (mode) {
    case 1:  return "worm";
    case 2:  return "big worm";
    default: return "no worm";
    }
}

static ccColor3B modeTint(int mode) {
    switch (mode) {
    case 1:  return { 255, 255, 255 };
    case 2:  return { 255, 220, 160 };
    default: return { 180, 180, 180 };
    }
}

static void applyHistoryForMode() {
    //normal worm is only off screen at 4x speed
    if (gWormMode == 1)
        gWormHistory = 90;
    //big worm goes off screen even at 0.5x speed
    else if (gWormMode == 2)
        gWormHistory = 200;
    else
        gWormHistory = 90;
}

class $modify(WormToggleMenu, MenuLayer) {
public:
    struct Fields {
        CCLabelBMFont* label = nullptr;
        CCSprite* buttonSprite = nullptr;
        CCMenuItemSpriteExtra* buttonItem = nullptr;
    };

    bool init() {
        if (!MenuLayer::init()) return false;

        //get bottom menu for worm button
        auto* bottomNode = this->getChildByID("bottom-menu");
        auto* bottomMenu = typeinfo_cast<CCMenu*>(bottomNode);
        if (!bottomMenu) {
            bottomMenu = CCMenu::create();
            bottomMenu->setPosition({ 0.f, 0.f });
            bottomMenu->setID("bottom-menu");
            this->addChild(bottomMenu, 10);
        }

        //button sprite I made for worm button
        CCSprite* base = nullptr;
        if (auto* s = CCSprite::create("button.png"_spr)) {
            base = s;
        }
        else {
            base = CCSprite::create();
            base->setContentSize({ 120.f, 48.f });
        }
        applyHistoryForMode();

        //worm button label
        auto* label = CCLabelBMFont::create(modeLabel(gWormMode), "bigFont.fnt");
        label->setScale(0.27f);
        label->setPosition({
            base->getContentSize().width / 2.f,
            base->getContentSize().height / 2.f
            });
        base->addChild(label);

        m_fields->label = label;
        m_fields->buttonSprite = base;

        //make worm button clickable
        auto* item = CCMenuItemSpriteExtra::create(base, this, menu_selector(WormToggleMenu::onWormButton));
        item->setID("worm-toggle-button"_spr);

        m_fields->buttonItem = item;
        bottomMenu->addChild(item);
        bottomMenu->updateLayout();

        //set tint depending on current mode
        if (m_fields->buttonSprite)
            m_fields->buttonSprite->setColor(modeTint(gWormMode));

        return true;
    }

    //toggle between no worm/worm/big worm
    void onWormButton(CCObject*) {
        gWormMode = (gWormMode + 1) % 3;
        applyHistoryForMode();

        //update label text
        if (m_fields->label)
            m_fields->label->setString(modeLabel(gWormMode));

        //update button tint
        if (m_fields->buttonSprite)
            m_fields->buttonSprite->setColor(modeTint(gWormMode));
    }
};
