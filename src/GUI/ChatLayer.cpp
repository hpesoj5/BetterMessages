#include "ChatLayer.hpp"

$on_mod(Loaded) {
    ChatLayer::get();
}

Ref<ChatLayer> ChatLayer::get() {
    static Ref<ChatLayer> chatLayer { create() };
    return chatLayer;
}

ChatLayer* ChatLayer::create() {
    auto ptr { new ChatLayer };
    if (ptr && ptr->init()) {
        log::info("Instance created");
        ptr->retain();
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
    if (!CCLayerColor::initWithColor({ 0, 0, 0, 127 }, winSize.width, winSize.height)) return false;

    setKeypadEnabled(true);
    // setTouchEnabled(true);
    setKeyboardEnabled(true);

    return true;
}

void ChatLayer::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true);
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
    // CCTouchDispatcher::get()->registerForcePrio(this, 2);
    m_isOpen = true;
}

void ChatLayer::onClose(CCObject*) {
    close();
}

void ChatLayer::close() {
    log::info("Close instance");
    // CCTouchDispatcher::get()->unregisterForcePrio(this);
    this->removeFromParent();
    m_isOpen = false;
}
