#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class ProfileButton final : public CCMenu {
    public:
        static ProfileButton* create(float width, float height, GJUserScore* user, CCObject* target, SEL_MenuHandler selector);

        void updateZOrder(int zOrder);

    private:
        ProfileButton() = default;
        ~ProfileButton() = default;

        bool init(float width, float height, GJUserScore* user, CCObject* target, SEL_MenuHandler selector);

        CCMenuItemSpriteExtra* m_button;
        SimplePlayer* m_icon;
        CCLabelBMFont* m_name;
    };
}
