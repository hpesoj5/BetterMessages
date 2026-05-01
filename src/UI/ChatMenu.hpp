#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class ChatMenu final : public CCMenu {
    public:
        static Ref<ChatMenu> get();

    private:
        ChatMenu() = default;
        ~ChatMenu();

        ChatMenu(ChatMenu const&) = delete;
        ChatMenu(ChatMenu&&) = delete;
        ChatMenu& operator=(ChatMenu const&) = delete;
        ChatMenu& operator=(ChatMenu&&) = delete;

        static ChatMenu* create();
        bool init() override;
    };
}
