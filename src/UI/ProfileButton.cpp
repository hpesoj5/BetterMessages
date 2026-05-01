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
        setContentSize({ width, height });

        auto background { NineSlice::create("GJ_button_05.png") };
        background->setContentSize({ width, height });
        if (!Button::init(background, nullptr, nullptr, target, selector)) return false;

        auto [contentWidth, contentHeight] { getContentSize() };
        auto zOrder { getZOrder() };


        // init icon
        m_icon = SimplePlayer::create(user->m_iconID);
        m_icon->updatePlayerFrame(user->m_iconID, user->m_iconType);
        m_icon->setAnchorPoint({ 0.f, 0.5f });
        m_icon->setPosition({  contentWidth * 0.15f, contentHeight / 2.f });
        auto gm { GameManager::get() };
        m_icon->setColors(gm->colorForIdx(user->m_color1), gm->colorForIdx(user->m_color2));

        log::info("User {}: glow enabled: {}", user->m_userName, user->m_glowEnabled);
        if (user->m_glowEnabled) m_icon->setGlowOutline(gm->colorForIdx(user->m_color3));
        else m_icon->disableGlowOutline();

        m_icon->setScale(0.7f);

        addChild(m_icon, zOrder + 1);

        // init player name
        m_name = CCLabelBMFont::create(user->m_userName.c_str(), "bigFont.fnt");
        m_name->setAnchorPoint({ 0.f, 0.5f });
        m_name->setPosition({ contentWidth * 0.27f, contentHeight / 2.f });
        m_name->setScale(0.4f);

        addChild(m_name, zOrder + 1);

        updateLayout();
        return true;
    }

    void ProfileButton::updateZOrder(int zOrder) {
        setZOrder(zOrder);
        m_icon->setZOrder(zOrder + 1);
        m_name->setZOrder(zOrder + 1);
    }
}
