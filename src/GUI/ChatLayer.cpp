#include "ChatLayer.hpp"
#include "ProfileSelectMenu.hpp"
#include "ChatMenu.hpp"
#include "ChatHandler.hpp"

namespace BetterMessages {
    Ref<ChatLayer> ChatLayer::get() {
        static Ref<ChatLayer> chatLayer { create() };
        return chatLayer;
    }

    ChatLayer* ChatLayer::create() {
        auto ptr { new ChatLayer };
        if (ptr && ptr->init()) {
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    ChatLayer::ChatLayer() : m_isOpen {} {}

    ChatLayer::~ChatLayer() {}

    bool ChatLayer::init() {
        auto winSize { CCDirector::get()->getWinSize() };
        if (!Popup::initWithColor({ 0, 0, 0, 223 }, winSize.width, winSize.height)) return false;

        setKeypadEnabled(true);
        setTouchEnabled(true);
        setKeyboardEnabled(true);

        setID("ChatLayer"_spr);
        setUserFlag("alk.better-touch-prio/steals-touch");

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
        auto zOrder { scene->getHighestChildZ() + 1 };
        scene->addChild(this, zOrder);

        ProfileSelectMenu::get()->updateZOrder(zOrder + 1);

        auto ch { ChatMenu::get() };
        ChatHandler::get()->refreshChat(ChatHandler::get()->getActiveUserID());
        if (ChatHandler::get()->isLoading()) ch->setLoadingSpinner(true);
        ch->updateZOrder(zOrder + 1);

        m_isOpen = true;
        log::info("opened");
        m_openNotif.notifyAll();
    }

    void ChatLayer::onClose(CCObject*) {
        close();
    }

    void ChatLayer::close() {
        ProfileSelectMenu::get()->resetInput();
        ProfileSelectMenu::get()->retrieveFriends();
        ChatMenu::get()->defocus();
        ChatMenu::get()->setLoadingSpinner(false);
        this->removeFromParent();
        ChatHandler::get()->saveToDisk();

        m_isOpen = false;
    }
}
