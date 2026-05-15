#include "ChatHandler.hpp"
#include "ChatMenu.hpp"
#include "Serialisation.hpp"
#include <algorithm>
#include <utility>

namespace BetterMessages {
    ChatHandler* ChatHandler::get() {
        static ChatHandler handler;
        return &handler;
    }

    ChatHandler::~ChatHandler() { saveToDisk(); }

    void ChatHandler::saveToDisk() {
        if (m_loadMtx.tryLock()) {
            auto m { Mod::get() };
            m->setSavedValue("chatData", m_chats);
            m->setSavedValue("highestReceivedMessageID", m_highestReceivedMessageID);
            m->setSavedValue("highestSentMessageID", m_highestSentMessageID);
        }
    }

    void ChatHandler::restoreFromDisk() {
        async::spawn([this] -> arc::Future<> {
            co_await m_loadMtx.lock();
            co_await async::waitForMainThread([this] {
                auto m { Mod::get() };
                m_chats = m->getSavedValue<std::unordered_map<int, Chat>>("chatData");
                m_highestReceivedMessageID = m->getSavedValue<int>("highestReceivedMessageID");
                m_highestSentMessageID = m->getSavedValue<int>("highestSentMessageID");
            });
        });
    }

    int ChatHandler::getActiveUserID() const { return m_activeUserID.empty() ? -1 : m_activeUserID.back(); }

    void ChatHandler::setActiveUserID(int userID) {
        if (userID == -1) return;
        removeUserID(userID);
        m_activeUserID.push_back(userID);
    }

    void ChatHandler::removeUserID(int userID) {
        if (userID == -1 || m_activeUserID.empty()) return;
        if (m_activeUserID.back() == userID) m_activeUserID.pop_back();
        else {
            auto it { std::find(m_activeUserID.begin(), m_activeUserID.end(), userID) };
            if (it != m_activeUserID.end()) m_activeUserID.erase(it);
        }
    }

    void ChatHandler::saveChat(int userID) {
        auto it { m_chats.find(userID) };
        if (it == m_chats.end()) return;

        it->second.unreadCount = 0;
        it->second.draftMessage = ChatMenu::get()->getInputString();
    }

    void ChatHandler::restoreChat(int userID) {
        if (userID == -1) {
            ChatMenu::get()->setInputString("");
            ChatMenu::get()->displayChatHistory({});
            return;
        }

        auto const& history { m_chats[userID].history };  // if element does not exist, create it
        auto const& input { m_chats[userID].draftMessage };
        ChatMenu::get()->setInputString(input);
        ChatMenu::get()->displayChatHistory(history);
    }

    void ChatHandler::switchChat(int userID) {
        auto activeUserID { getActiveUserID() };

        if (activeUserID != -1 && activeUserID != userID) {
            saveChat(activeUserID);
            auto button { ChatMenu::get()->getTabButtonByTag(activeUserID) };
            if (button) button->setSelectedSprite(false);
        }

        setActiveUserID(userID);
        activeUserID = userID;
        auto button { ChatMenu::get()->getTabButtonByTag(userID) };
        if (button) button->setSelectedSprite(true);
        restoreChat(userID);
    }

    void ChatHandler::refreshChat(int userID) {
        if (m_isRefreshing) return;
        m_isRefreshing = true;
        auto activeUserID { getActiveUserID() };

        if (userID == -1 || activeUserID != userID) {
            m_isRefreshing = false;
            return;
        }

        // async::spawn(downloadChat(userID), [this, userID] {
            auto const& history { m_chats[userID].history };
            ChatMenu::get()->displayChatHistory(history);
            m_isRefreshing = false;
        // });
    }

    // arc::Future<> ChatHandler::loadMessages() {

    // }

    void ChatHandler::sortChats() {
        for (auto& [userID, chat] : m_chats) {
            std::sort(chat.history.begin(), chat.history.end(), [](Ref<GJUserMessage> const& a, Ref<GJUserMessage> const& b) { return a->m_messageID < b->m_messageID; });
        }
    }

    // arc::Future<> ChatHandler::sendMessage(int userID, std::string content, std::string subject) {

    // }
}
