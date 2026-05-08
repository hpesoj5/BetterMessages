#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class ChatLayer final : public Popup {
    public:
        static Ref<ChatLayer> get();

        bool isOpen() { return m_isOpen; }

        void keyBackClicked() override;

        void toggleOpen();
        void open();
        void onClose(CCObject*) override;  // for close button
        void close();

        arc::Notify m_openNotif;

    private:
        ChatLayer();
        ~ChatLayer();

        ChatLayer(ChatLayer const&) = delete;
        ChatLayer(ChatLayer&&) = delete;
        ChatLayer& operator=(ChatLayer const&) = delete;
        ChatLayer& operator=(ChatLayer&&) = delete;

        static ChatLayer* create();
        bool init() override;

        // NineSlice* m_background;

        int m_zLayer;
        bool m_isOpen;
    };
}
