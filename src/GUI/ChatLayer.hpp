#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ChatLayer final : public CCLayerColor {
public:
    static Ref<ChatLayer> get();

    bool isOpen() { return m_isOpen; }

    void toggleOpen();
    void open();
    void onClose(CCObject*);  // for close button
    void close();

private:
    ChatLayer();
    ~ChatLayer();

    static ChatLayer* create();
    bool init() override;

    bool m_isOpen;
};
