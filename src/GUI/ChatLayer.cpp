#include "ChatLayer.hpp"
#include "ProfileSelectMenu.hpp"
#include "ChatMenu.hpp"


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
        if (!Popup::initWithColor({ 0, 0, 0, 191 }, winSize.width, winSize.height)) return false;

        setKeypadEnabled(true);
        setTouchEnabled(true);
        setKeyboardEnabled(true);

        setID("ChatLayer"_spr);
        setUserFlag("alk.better-touch-prio/steals-touch");

        // m_background = NineSlice::create("GJ_square05.png");
        // m_background->setAnchorPoint({ 0.f, 0.f });
        // m_background->setContentSize(winSize);

        // addChild(m_background);
        addChild(ProfileSelectMenu::get());
        addChild(ChatMenu::get());
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
        auto zOrder { scene->getHighestChildZ() + 1 };
        scene->addChild(this, zOrder);

        // m_background->setZOrder(zOrder);

        ProfileSelectMenu::get()->updateZOrder(zOrder + 1);

        ChatMenu::get()->updateZOrder(zOrder + 1);
        m_isOpen = true;
    }

    void ChatLayer::onClose(CCObject*) {
        close();
    }

    void ChatLayer::close() {
        log::info("Close instance");
        ProfileSelectMenu::get()->resetInput();
        ProfileSelectMenu::get()->retrieveFriends();
        this->removeFromParent();

        m_isOpen = false;
    }
}
