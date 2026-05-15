// TODO: change delegates to http requests to avoid conflict with GD's message delegates

#pragma once

#include "TabButton.hpp"
#include <Geode/Geode.hpp>
#include <arc/sync/Mutex.hpp>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

using namespace geode::prelude;

namespace BetterMessages {
    struct Chat {
        std::vector<Ref<GJUserMessage>> history {};
        std::string draftMessage {};
        int unreadCount {};
    };
}
namespace BetterMessages {
    class ChatHandler final {
    public:
        static ChatHandler* get();

        void switchChat(int userID);
        void refreshChat(int userID);
        void sendMessage(int userID, std::string content, std::string subject = "Sent with BetterMessages");

        void loadMessages();

        void saveToDisk();
        void restoreFromDisk();

        int getActiveUserID() const;
        void setActiveUserID(int userID);
        void removeUserID(int userID);

    private:
        ChatHandler() = default;
        ~ChatHandler();

        ChatHandler(ChatHandler const& other) = delete;
        ChatHandler(ChatHandler&& other) = delete;
        ChatHandler& operator=(ChatHandler const& other) = delete;
        ChatHandler& operator=(ChatHandler&& other) = delete;

        void parseMessageString(std::string const& data);
        void sortChats();

        friend void TabButton::onClose(CCObject*);
        void saveChat(int userID);
        void restoreChat(int userID);

        arc::Mutex<int> m_mtx;

        std::unordered_map<int, Chat> m_chats;
        Ref<GJUserMessage> m_downloadedMessage;

        std::vector<int> m_activeUserID {};
        int m_highestSentMessageID {};
        int m_highestReceivedMessageID {};
        int m_temporarySentID {};
        int m_temporaryReceivedID {};
        bool m_stopLoading {};
        bool m_isLoading {};
    };
}
