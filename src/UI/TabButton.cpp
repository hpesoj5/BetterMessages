#include "Helpers.hpp"
#include "TabButton.hpp"

namespace BetterMessages {
    TabButton* TabButton::create(float width, float height, GJUserScore* user) {
        auto ptr { new TabButton };
        if (ptr && ptr->init(width, height, user)) {
            ptr->autorelease();
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    bool TabButton::init(float width, float height, GJUserScore* user) {
        if (!CCMenu::init()) return false;

        setID(user->m_userName);
        setContentSize({ width, height });
        auto zOrder { getZOrder() };

        NineSlice::Insets insets { 5.f, 5.f, 0.f,5.f };
        auto background { NineSlice::createWithSpriteFrameName("GJ_tabOff_001.png", insets) };
        background->setContentSize({ width, height });
        auto backgroundSelected { NineSlice::createWithSpriteFrameName("GJ_tabOn_001.png", insets) };
        backgroundSelected->setContentSize({ width, height });

        // init tab
        m_button = CCMenuItemSpriteExtra::create(background, backgroundSelected, this, menu_selector(TabButton::select));
        m_button->setContentSize({ width, height });
        m_button->setAnchorPoint({ 0.5f, 0.5f });
        m_button->setPosition(width / 2.f, height / 2.f);
        m_button->m_animationEnabled = false;

        addChild(m_button, zOrder + 1);

        // init tab name
        m_name = CCLabelBMFont::create(user->m_userName.c_str(), "bigFont.fnt");
        m_name->setAnchorPoint({ 0.f, 0.5f });
        m_name->setPosition(width * 0.1f, height * 0.45f);
        m_name->setScale(getScaleFromLength(user->m_userName.size()) / 2.f);

        m_button->addChild(m_name, zOrder + 2);

        // init close
        m_closeMenu = CCMenu::create();
        m_closeMenu->setAnchorPoint({ 0.5f, 0.5f });
        m_closeMenu->setPosition(width * 0.88f, height * 0.45f);
        addChild(m_closeMenu, zOrder + 2);

        m_close = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"), this, menu_selector(TabButton::onClose));
        m_close->setScale(0.3f);
        m_close->m_animationEnabled = false;

        m_closeMenu->addChild(m_close, zOrder + 3);
        m_closeMenu->updateLayout();

        updateLayout();
        return true;
    }

    void TabButton::updateZOrder(int zOrder) {
        setZOrder(zOrder);
        m_button->setZOrder(zOrder + 1);
        m_name->setZOrder(zOrder + 2);
        m_closeMenu->setZOrder(zOrder + 2);
        m_close->setZOrder(zOrder + 3);

        m_closeMenu->updateLayout();
        updateLayout();
    }

    void TabButton::onClose(CCObject*) {
        auto par { this->getParent() };
        this->removeFromParent();
        par->updateLayout();
    }

    void TabButton::select(CCObject*) {
        // m_button->selected();
    }
}
