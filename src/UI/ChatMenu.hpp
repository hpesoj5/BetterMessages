#pragma once

#include <Geode/Geode.hpp>
#include "TabButton.hpp"

using namespace geode::prelude;

namespace BetterMessages {
    class ChatMenu final : public CCMenu, public TextInputDelegate {
    public:
        static Ref<ChatMenu> get();

        void setLoadingSpinner(bool visible);

        void updateZOrder(int zOrder);
        void goToUser(GJUserScore* user);
        void defocus();

        std::string getInputString() const;
        void setInputString(std::string const& draft);

        void restoreChatHistory(std::vector<Ref<GJUserMessage>> const& history);

        TabButton* getTabButtonByTag(int tag);

    private:
        ChatMenu() = default;
        ~ChatMenu();

        ChatMenu(ChatMenu const&) = delete;
        ChatMenu(ChatMenu&&) = delete;
        ChatMenu& operator=(ChatMenu const&) = delete;
        ChatMenu& operator=(ChatMenu&&) = delete;

        static ChatMenu* create();
        bool init() override;

        void textInputOpened(CCTextInputNode* node) override;
        void textInputClosed(CCTextInputNode* node) override;
        void enterPressed(CCTextInputNode* node) override;

        CCMenu* m_tabButtonMenu;
        NineSlice* m_tabButtonMenuBG;

        ScrollLayer* m_chatHistoryLayer;
        TextInput* m_chatInput;

        LoadingSpinner* m_chatLoadingSpinner;

        bool m_focused {};
    };
}
