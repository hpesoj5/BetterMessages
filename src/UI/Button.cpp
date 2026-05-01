#include "Button.hpp"

namespace BetterMessages {
    Button* Button::create(CCNode* normalSprite, CCObject* target, SEL_MenuHandler selector) {
        return create(normalSprite, nullptr, nullptr, target, selector);
    }

    Button* Button::create(CCNode* normalSprite, CCNode* selectedSprite, CCObject* target, SEL_MenuHandler selector) {
        return create(normalSprite, selectedSprite, nullptr, target, selector);
    }

    Button* Button::create(CCNode* normalSprite, CCNode* selectedSprite, CCNode* disabledSprite, CCObject* target, SEL_MenuHandler selector) {
        auto ptr { new Button };
        if (ptr && ptr->init(normalSprite, selectedSprite, disabledSprite, target, selector)) {
            ptr->autorelease();
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    bool Button::init(CCNode* normalSprite, CCNode* selectedSprite, CCNode* disabledSprite, CCObject* target, SEL_MenuHandler selector) {
        if (!CCMenuItemSprite::initWithNormalSprite(normalSprite, selectedSprite, disabledSprite, target, selector)) return false;

        return true;
    }

    void Button::setRepeatEnabled(bool enabled) { m_repeatEnabled = enabled; }

    bool Button::isRepeatEnabled() const { return m_repeatEnabled; }

    void Button::draw() {}
    void Button::update(float dt) { }
}
