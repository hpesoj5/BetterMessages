#include "Constants.hpp"
#include "ChatLayer.hpp"
#include "ProfileSelectMenu.hpp"


$on_mod(Loaded) {
    BetterMessages::ChatLayer::get();
}

namespace BetterMessages {
    Ref<ChatLayer> ChatLayer::get() {
        static Ref<ChatLayer> chatLayer { create() };
        return chatLayer;
    }

    ChatLayer* ChatLayer::create() {
        auto ptr { new ChatLayer };
        if (ptr && ptr->init()) {
            log::info("Instance created");
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    ChatLayer::ChatLayer() : m_isOpen {} {}

    ChatLayer::~ChatLayer() {
        log::info("Instance destroyed");
    }

    bool ChatLayer::init() {
        auto winSize { CCDirector::get()->getWinSize() };
        if (!Popup::initWithColor({ 0, 0, 0, 127 }, winSize.width, winSize.height)) return false;

        setKeypadEnabled(true);
        setTouchEnabled(true);
        setKeyboardEnabled(true);

        setID("ChatLayer"_spr);
        setUserFlag("alk.better-touch-prio/steals-touch");

        ProfileSelectMenu::get()->setPosition({ Constants::ChatLayer::PADDING, winSize.height / 2.f });
        return true;
    }

    void ChatLayer::keyBackClicked() {
        if (m_isOpen) close();
    }

    void ChatLayer::toggleOpen() {
        m_isOpen ? close() : open();
    }

    void ChatLayer::open() {
        auto scene { CCScene::get() };
        log::info("Open instance");
        scene->addChild(this, scene->getHighestChildZ() + 1);

        addChild(ProfileSelectMenu::get(), getZOrder() + 1);
        ProfileSelectMenu::get()->updateZOrder();
        m_isOpen = true;
    }

    void ChatLayer::onClose(CCObject*) {
        close();
    }

    void ChatLayer::close() {
        log::info("Close instance");
        ProfileSelectMenu::get()->defocusInput();
        this->removeFromParent();
        ProfileSelectMenu::get()->removeFromParent();
        m_isOpen = false;
    }
}
