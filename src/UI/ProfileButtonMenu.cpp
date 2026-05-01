#include "ProfileButton.hpp"
#include "ProfileButtonMenu.hpp"

namespace BetterMessages {
    ProfileButtonMenu* ProfileButtonMenu::create() {
        auto ptr { new ProfileButtonMenu };
        if (ptr && ptr->init()) {
            ptr->autorelease();
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    ProfileButtonMenu* ProfileButtonMenu::create(CCArray* users) {
        auto ptr { new ProfileButtonMenu };
        if (ptr && ptr->init(users)) {
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    bool ProfileButtonMenu::init() {
        setLayout(RowLayout::create()->setGrowCrossAxis(true)->setGap(0.f));
        setID("ProfileButtonMenu"_spr);
        return CCMenu::init();
    }

    bool ProfileButtonMenu::init(CCArray* users) {
        if (!CCMenu::init()) return false;

        setLayout(RowLayout::create()->setGrowCrossAxis(true)->setGap(0.f));
        setID("ProfileButtonMenu"_spr);
        updateUsers(users);

        return true;
    }

    void ProfileButtonMenu::updateUsers(CCArray* users) {
        log::info("Friends: {}", users);
        log::info("# of Children: {}", getChildrenCount());
        if (getChildrenCount() > 0) removeAllChildren();

        auto size { users->count() };
        auto [contentWidth, contentHeight] { getContentSize() };
        for (auto i { 0uz }; i < size; ++i) {
            auto user { static_cast<GJUserScore*>(users->objectAtIndex(i)) };
            auto button { ProfileButton::create(contentWidth / 4.f, contentHeight / 4.f, user, this, nullptr) };
            button->setID(user->m_userName);
            log::info("User {}: {}. Button: {}", i, user->m_userName, button);
            addChild(button);  // crashes here
        }

        updateLayout();
    }

    void ProfileButtonMenu::updateZOrder(int zOrder) {
        setZOrder(zOrder);
        auto size { getChildrenCount() };
        for (auto i { 0uz }; i < size; ++i) {
            static_cast<ProfileButton*>(getChildByIndex(i))->updateZOrder(zOrder + 1);
        }

        updateLayout();
    }
}
