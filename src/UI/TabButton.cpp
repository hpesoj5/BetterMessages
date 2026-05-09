#include "Helpers.hpp"
#include "TabButton.hpp"
#include "ChatHandler.hpp"

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
        setTag(user->m_userID);
        setContentSize({ width, height });
        auto zOrder { getZOrder() };

        NineSlice::Insets insets { 5.f, 5.f, 0.f,5.f };
        auto background { NineSlice::createWithSpriteFrameName("GJ_tabOn_001.png", insets) };
        background->setContentSize({ width, height });
        background->setOpacity(127);
        auto backgroundSelected { NineSlice::createWithSpriteFrameName("GJ_tabOn_001.png", insets) };
        backgroundSelected->setContentSize({ width, height });

        m_selectedSprite = NineSlice::createWithSpriteFrameName("GJ_tabOn_001.png", insets);
        m_selectedSprite->setAnchorPoint({ 0.5f, 0.5f });
        m_selectedSprite->setContentSize({ width, height });
        m_selectedSprite->setPosition(width / 2.f, height / 2.f);
        m_selectedSprite->setVisible(false);

        // init tab
        m_button = CCMenuItemSpriteExtra::create(background, backgroundSelected, this, menu_selector(TabButton::select));
        m_button->setContentSize({ width, height });
        m_button->setAnchorPoint({ 0.5f, 0.5f });
        m_button->setPosition(width / 2.f, height / 2.f);
        m_button->m_animationEnabled = false;
        m_button->setTag(user->m_userID);

        addChild(m_button, zOrder + 1);
        m_button->addChild(m_selectedSprite, zOrder + 2);

        // init tab name
        m_name = CCLabelBMFont::create(user->m_userName.c_str(), "bigFont.fnt");
        m_name->setAnchorPoint({ 0.f, 0.5f });
        m_name->setPosition(width * 0.1f, height * 0.45f);
        m_name->setScale(getScaleFromLength(user->m_userName.size()) / 2.f);

        m_button->addChild(m_name, zOrder + 3);

        // init close
        m_closeMenu = CCMenu::create();
        m_closeMenu->setAnchorPoint({ 0.5f, 0.5f });
        m_closeMenu->setPosition(width * 0.88f, height * 0.45f);
        addChild(m_closeMenu, zOrder + 3);

        m_close = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"), this, menu_selector(TabButton::onClose));
        m_close->setScale(0.3f);
        m_close->m_animationEnabled = false;
        m_close->setTag(user->m_userID);

        m_closeMenu->addChild(m_close, zOrder + 4);
        m_closeMenu->updateLayout();

        updateLayout();
        return true;
    }

    void TabButton::updateZOrder(int zOrder) {
        setZOrder(zOrder);
        m_button->setZOrder(zOrder + 1);
        m_selectedSprite->setZOrder(zOrder + 2);
        m_name->setZOrder(zOrder + 3);
        m_closeMenu->setZOrder(zOrder + 3);
        m_close->setZOrder(zOrder + 4);

        m_closeMenu->updateLayout();
        updateLayout();
    }

    void TabButton::onClose(CCObject* sender) {
        auto par { this->getParent() };
        this->removeFromParent();
        par->updateLayout();
        auto userID { sender->getTag() };
        log::info("Chat {} closed", userID);
        auto ch { ChatHandler::get() };
        auto activeUserID { ch->getActiveUserID() };
        ch->removeUserID(userID);
        if (userID == activeUserID) ch->switchChat(ch->getActiveUserID());
    }

    void TabButton::select(CCObject* sender) {
        auto userID { sender->getTag() };
        log::info("Switched to {}", userID);
        ChatHandler::get()->switchChat(userID);
    }

    void TabButton::setSelectedSprite(bool selected) {
        m_selectedSprite->setVisible(selected ? true : false);
        setOpacity(selected ? 255 : 127);
    }
}
