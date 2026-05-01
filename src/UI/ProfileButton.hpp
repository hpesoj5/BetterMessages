#pragma once

#include "Button.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class ProfileButton final : public Button {
    public:
        static ProfileButton* create(float width, float height, GJUserScore* user, CCObject* target, SEL_MenuHandler selector);

        void updateZOrder(int zOrder);

    private:
        ProfileButton() = default;

        bool init(float width, float height, GJUserScore* user, CCObject* target, SEL_MenuHandler selector);

        // void draw() override;

        SimplePlayer* m_icon;
        CCLabelBMFont* m_name;
    };
}
