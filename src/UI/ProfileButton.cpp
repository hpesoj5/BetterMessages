#include "Helpers.hpp"
#include "ProfileButton.hpp"

namespace BetterMessages {
    ProfileButton* ProfileButton::create(float width, float height, GJUserScore* user, CCObject* target, SEL_MenuHandler selector) {
        auto ptr { new ProfileButton };
        if (ptr && ptr->init(width, height, user, target, selector)) {
            ptr->autorelease();
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    bool ProfileButton::init(float width, float height, GJUserScore* user, CCObject* target, SEL_MenuHandler selector) {
        if (!CCMenu::init()) return false;
        setContentSize({ width, height });
        auto zOrder { getZOrder() };

        auto background { NineSlice::create("square02_small.png") };
        background->setOpacity(0);
        background->setContentSize({ width, height });
        auto backgroundSelected { NineSlice::create("square02b_small.png") };
        backgroundSelected->setOpacity(50);
        backgroundSelected->setContentSize({ width, height });

        m_button = CCMenuItemSpriteExtra::create(background, backgroundSelected, target, selector);
        m_button->setContentSize({ width, height });
        m_button->setAnchorPoint({ 0.5f, 0.5f });
        m_button->setPosition({ width / 2.f, height / 2.f });
        m_button->m_animationEnabled = false;
        m_button->setID(user->m_userName);
        m_button->setUserObject(user);
        addChild(m_button, zOrder + 1);

        // init icon
        m_icon = SimplePlayer::create(user->m_iconID);
        m_icon->updatePlayerFrame(user->m_iconID, user->m_iconType);
        m_icon->setAnchorPoint({ 0.f, 0.5f });
        m_icon->setPosition({  width * 0.15f, height / 2.f });
        auto gm { GameManager::get() };
        m_icon->setColors(gm->colorForIdx(user->m_color1), gm->colorForIdx(user->m_color2));

        // log::info("User {}: glow enabled: {}", user->m_userName, user->m_glowEnabled);
        if (user->m_glowEnabled) m_icon->setGlowOutline(gm->colorForIdx(user->m_color3));
        else m_icon->disableGlowOutline();

        m_icon->setScale(0.6f);

        m_button->addChild(m_icon, zOrder + 2);

        // init player name
        m_name = CCLabelBMFont::create(user->m_userName.c_str(), "bigFont.fnt");
        m_name->setAnchorPoint({ 0.f, 0.5f });
        m_name->setPosition({ width * 0.27f, height / 2.f });
        m_name->setScale(getScaleFromLength(user->m_userName.size()));

        m_button->addChild(m_name, zOrder + 2);

        updateLayout();
        return true;
    }

    void ProfileButton::updateZOrder(int zOrder) {
        setZOrder(zOrder);
        m_button->setZOrder(zOrder + 1);
        m_icon->setZOrder(zOrder + 2);
        m_name->setZOrder(zOrder + 2);

        updateLayout();
    }
}
