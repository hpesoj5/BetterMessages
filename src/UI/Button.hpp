#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// base class referenced from QOLmod
namespace BetterMessages {
    class Button : public CCMenuItemSprite {
    public:
        static Button* create(CCNode* normalSprite, CCObject* target, SEL_MenuHandler selector);
        static Button* create(CCNode* normalSprite, CCNode* selectedSprite, CCObject* target, SEL_MenuHandler selector);
        static Button* create(CCNode* normalSprite, CCNode* selectedSprite, CCNode* disabledSprite, CCObject* target, SEL_MenuHandler selector);

        void setRepeatEnabled(bool enabled);
        bool isRepeatEnabled() const;

        // void selected() override;
        // void unselected() override;
        // void activate() override;

    protected:
        bool init(CCNode* normalSprite, CCNode* selectedSprite, CCNode* disabledSprite, CCObject* target, SEL_MenuHandler selector);

        void draw() override;
        void update(float dt) override;

        bool m_repeatEnabled { false };
        float t {};
        float v {};
        float d {};
    };
}
