//ui_worm.cpp
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

//default setting, no worm :(
bool gWormOn = false;

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

        //worm button label
        auto* label = CCLabelBMFont::create(gWormOn ? "worm" : "no worm", "bigFont.fnt");
        label->setScale(0.3f);
        label->setPosition({
            base->getContentSize().width / 2.f,
            base->getContentSize().height / 2.f
            });
        base->addChild(label);

        m_fields->label = label;
        m_fields->buttonSprite = base;

        //worm button clickable
        auto* item = CCMenuItemSpriteExtra::create(base, this, menu_selector(WormToggleMenu::onWormButton));
        item->setID("worm-toggle-button"_spr);

        m_fields->buttonItem = item;
        bottomMenu->addChild(item);
        bottomMenu->updateLayout();

        //worm button tint for on/off
        if (m_fields->buttonSprite)
            m_fields->buttonSprite->setColor(gWormOn ? ccColor3B{ 255,255,255 } : ccColor3B{ 180,180,180 });

        return true;
    }

    void onWormButton(CCObject*) {
        gWormOn = !gWormOn;

        if (m_fields->label)
            m_fields->label->setString(gWormOn ? "worm" : "no worm");

        if (m_fields->buttonSprite)
            m_fields->buttonSprite->setColor(gWormOn ? ccColor3B{ 255,255,255 } : ccColor3B{ 180,180,180 });
    }
};
