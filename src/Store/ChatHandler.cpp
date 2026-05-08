#include "ChatHandler.hpp"
#include "ChatLayer.hpp"
#include "ChatMenu.hpp"
#include "Constants.hpp"
#include <Geode/utils/coro.hpp>
#include <algorithm>
#include <utility>

namespace BetterMessages {
    ChatHandler* ChatHandler::get() {
        static ChatHandler handler;
        return &handler;
    }

    bool ChatHandler::isLoading() const { return m_isLoading; }

    int ChatHandler::getActiveUserID() const { return m_activeUserID; }

    void ChatHandler::saveChat(int userID) {
        auto it { m_chats.find(userID) };
        if (it == m_chats.end()) return;

        it->second.unreadCount = 0;
        it->second.draftMessage = ChatMenu::get()->getInputString();
    }

    void ChatHandler::restoreChat(int userID) {
        if (userID == -1) {
            ChatMenu::get()->setInputString("");
            ChatMenu::get()->restoreChatHistory({});
            return;
        }

        auto const& history { m_chats[userID].history };  // if element does not exist, create it
        auto const& input { m_chats[userID].draftMessage };
        ChatMenu::get()->setInputString(input);
        ChatMenu::get()->restoreChatHistory(history);
    }

    void ChatHandler::switchChat(int userID) {
        if (m_activeUserID != -1) {
            saveChat(m_activeUserID);
            auto button { ChatMenu::get()->getTabButtonByTag(m_activeUserID) };
            if (button) button->setSelectedSprite(false);
        }
        m_activeUserID = userID;
        auto button { ChatMenu::get()->getTabButtonByTag(m_activeUserID) };
        if (button) button->setSelectedSprite(true);
        restoreChat(userID);
   }

    void ChatHandler::sendMessage(std::string content, std::string subject) {

    }

    arc::Future<> ChatHandler::loadMessages() {
        co_await m_loadMtx.lock();

        log::info("Fetching messages...");
        auto gm { GameLevelManager::get() };
        m_isLoading = true;

        prev_MLD = this;  // just in case
        std::swap(prev_MLD, gm->m_messageListDelegate);

        // unsent messages
        int page {};
        while (true) {
            gm->getUserMessages(false, page, 50);
            co_await m_loadNotif.notified();
            if (m_stopLoading) {
                m_stopLoading = false;
                break;
            }
            ++page;
        }

        // sent messages
        page = 0;
        while (true) {
            gm->getUserMessages(true, page, 50);
            co_await m_loadNotif.notified();
            if (m_stopLoading) {
                m_stopLoading = false;
                break;
            }
            ++page;
        }

        std::swap(prev_MLD, gm->m_messageListDelegate);
        m_isLoading = false;

        if (m_temporaryReceivedID > m_highestReceivedMessageID) m_highestReceivedMessageID = m_temporaryReceivedID;
        if (m_temporarySentID > m_highestSentMessageID) m_highestSentMessageID = m_temporarySentID;

        m_temporaryReceivedID = 0;
        m_temporarySentID = 0;

        sortChats();

        co_await downloadChats();
        log::info("Chats restored");
    }

    void ChatHandler::loadMessagesFinished(CCArray* messages, char const* key) {
        auto size { messages->count() };
        for (auto i { 0uz }; i < size; ++i) {
            auto message { static_cast<GJUserMessage*>(messages->objectAtIndex(i) ) };
            auto userID { message->m_userID };
            auto ID { message->m_messageID };
            auto sent { message->m_outgoing };
            if (sent) {
                if (ID <= m_highestSentMessageID) m_stopLoading = true;
                else {
                    log::info("New sent message fetched: {}\nRecipient: {}\nTime since: {}\nSubject: {}\nContent: {}", ID, message->m_username, message->m_uploadDate, message->m_title, message->m_content);
                    auto& chat { m_chats[userID] };
                    chat.history.push_back(message);
                }
                if (ID > m_temporarySentID) m_temporarySentID = ID;
            }
            else {
                if (message->m_messageID <= m_highestReceivedMessageID) m_stopLoading = true;
                else {
                    log::info("New received message fetched: {}\nSender: {}\nTime since: {}\nSubject: {}\nContent: {}", ID, message->m_username, message->m_uploadDate, message->m_title, message->m_content);
                    auto& chat { m_chats[userID] };
                    chat.history.push_back(message);
                    ++chat.unreadCount;
                }
                if (ID > m_temporaryReceivedID) m_temporaryReceivedID = ID;
            }
        }
        m_loadNotif.notifyAll();
    }

    void ChatHandler::loadMessagesFailed(char const* key, GJErrorCode errorType) {
        // -2: empty page
        if (static_cast<int>(errorType) == -2) m_stopLoading = true;
        m_loadNotif.notifyAll();
    }

    arc::Future<> ChatHandler::downloadChats() {
        for (auto& [userID, chat] : m_chats) {
            co_await downloadChat(userID);
        }
        sortChats();
    }

    arc::Future<> ChatHandler::downloadChat(int userID) {
        auto it { m_chats.find(userID) };
        if (it == m_chats.end()) co_return;

        co_await m_loadMtx.lock();  // don't wanna have loadMessages() change history while downloading chats
        co_await m_downloadMtx.lock();

        auto gm { GameLevelManager::get() };
        prev_DMD = this;  // just in case
        std::swap(prev_DMD, gm->m_downloadMessageDelegate);

        auto& history { it->second.history };
        for (auto message : history) {
            if (message->m_content.empty()) {
                gm->downloadUserMessage(message->m_messageID, message->m_outgoing);
                co_await m_downloadNotif.notified();
                message->m_content = m_downloadedMessage->m_content;
            }
        }

        std::swap(prev_DMD, gm->m_downloadMessageDelegate);
    }

    void ChatHandler::downloadMessageFinished(GJUserMessage* message) {
        m_downloadedMessage = message;
        m_downloadNotif.notifyAll();
    }

    void ChatHandler::downloadMessageFailed(int id) {
        log::info("Could not download message {}", id);
        m_downloadNotif.notifyAll();
    }

    void ChatHandler::sortChats() {
        for (auto& [userID, chat] : m_chats) {
            std::sort(chat.history.begin(), chat.history.end(), [](Ref<GJUserMessage> a, Ref<GJUserMessage> b) { return a->m_messageID < b->m_messageID; });
        }
    }

    void ChatHandler::uploadMessageFinished(int accountID) {

    }

    void ChatHandler::uploadMessageFailed(int accountID) {

    }
}
