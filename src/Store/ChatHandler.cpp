#include "ChatHandler.hpp"
#include "ChatLayer.hpp"
#include "ChatMenu.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <utility>

namespace BetterMessages {
    ChatHandler* ChatHandler::get() {
        static ChatHandler handler;
        return &handler;
    }

    bool ChatHandler::isLoading() const { return m_isLoading; }

    void ChatHandler::saveChat(int userID) {

    }

    void ChatHandler::restoreChat(int userID) {

    }

    void ChatHandler::switchChat(int userID) {

    }

    void ChatHandler::sendMessage(std::string content, std::string subject) {

    }

    arc::Future<> ChatHandler::loadMessages() {
        log::info("Fetching messages...");
        auto gm { GameLevelManager::get() };
        m_isLoading = true;
        std::swap(prev_MLD, gm->m_messageListDelegate);

        // unsent messages
        int page {};
        while (true) {
            gm->getUserMessages(false, page, 50);
            co_await m_notif.notified();
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
            co_await m_notif.notified();
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
        m_notif.notifyAll();
    }

    void ChatHandler::loadMessagesFailed(char const* key, GJErrorCode errorType) {
        // -2: empty page
        if (static_cast<int>(errorType) == -2) m_stopLoading = true;
        m_notif.notifyAll();
    }

    void ChatHandler::downloadChat(int userID) {

    }

    void ChatHandler::downloadMessages() {

    }

    void ChatHandler::downloadMessageFinished(GJUserMessage* message) {

    }

    void ChatHandler::downloadMessageFailed(int id) {

    }

    void ChatHandler::sortChats() {

    }

    void ChatHandler::uploadMessageFinished(int accountID) {

    }

    void ChatHandler::uploadMessageFailed(int accountID) {

    }
}
