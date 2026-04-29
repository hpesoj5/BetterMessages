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
    if (!CCLayerColor::init()) return false;

    // add init stuff
    return true;
}

void ChatLayer::toggleOpen() {
    m_isOpen ? close() : open();
}

void ChatLayer::open() {
    log::info("Open instance");
    m_isOpen = true;
}

void ChatLayer::onClose(CCObject*) {
    close();
}

void ChatLayer::close() {
    log::info("Close instance");
    m_isOpen = false;
}
