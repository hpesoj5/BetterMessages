#include "ChatHandler.hpp"
#include "ChatMenu.hpp"
#include "Serialisation.hpp"
#include <Geode/utils/coro.hpp>
#include <algorithm>
#include <utility>

namespace BetterMessages {
    ChatHandler* ChatHandler::get() {
        static ChatHandler handler;
        return &handler;
    }

    ChatHandler::~ChatHandler() { saveToDisk(); }

    void ChatHandler::saveToDisk() {
        Mod::get()->setSavedValue("chatData", m_chats);
    }

    void ChatHandler::restoreFromDisk() {
        m_chats = Mod::get()->getSavedValue<std::unordered_map<int, Chat>>("chatData");
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
            ChatMenu::get()->restoreChatHistory({});
            return;
        }

        auto const& history { m_chats[userID].history };  // if element does not exist, create it
        auto const& input { m_chats[userID].draftMessage };
        ChatMenu::get()->setInputString(input);
        ChatMenu::get()->restoreChatHistory(history);
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

        auto const& history { m_chats[userID].history };
        ChatMenu::get()->restoreChatHistory(history);
        m_isRefreshing = false;
    }

    arc::Future<> ChatHandler::loadMessages() {
        co_await m_loadMtx.lock();

        log::info("Fetching messages...");
        auto gm { GameLevelManager::get() };
        co_await async::waitForMainThread([] { ChatMenu::get()->setLoadingSpinner(true); });

        prev_MLD = this;  // just in case
        std::swap(prev_MLD, gm->m_messageListDelegate);

        // unsent messages
        int page {};
        while (true) {
            co_await async::waitForMainThread([gm, page] { gm->getUserMessages(false, page, 50); });
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
            co_await async::waitForMainThread([gm, page] { gm->getUserMessages(true, page, 50); });
            co_await m_loadNotif.notified();
            if (m_stopLoading) {
                m_stopLoading = false;
                break;
            }
            ++page;
        }

        std::swap(prev_MLD, gm->m_messageListDelegate);

        if (m_temporaryReceivedID > m_highestReceivedMessageID) m_highestReceivedMessageID = m_temporaryReceivedID;
        if (m_temporarySentID > m_highestSentMessageID) m_highestSentMessageID = m_temporarySentID;

        m_temporaryReceivedID = 0;
        m_temporarySentID = 0;

        co_await downloadChats();
        // log::info("Chats restored");
        co_await async::waitForMainThread([] { ChatMenu::get()->setLoadingSpinner(false); });
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
                    // log::info("New sent message fetched: {}\nRecipient: {}\nTime since: {}\nSubject: {}\nContent: {}", ID, message->m_username, message->m_uploadDate, message->m_title, message->m_content);
                    auto& chat { m_chats[userID] };
                    if (std::none_of(chat.history.begin(), chat.history.end(), [message](Ref<GJUserMessage> const& msg) { return message->m_messageID == msg->m_messageID; })) chat.history.push_back(message);
                }
                if (ID > m_temporarySentID) m_temporarySentID = ID;
            }
            else {
                if (message->m_messageID <= m_highestReceivedMessageID) m_stopLoading = true;
                else {
                    // log::info("New received message fetched: {}\nSender: {}\nTime since: {}\nSubject: {}\nContent: {}", ID, message->m_username, message->m_uploadDate, message->m_title, message->m_content);
                    auto& chat { m_chats[userID] };
                    if (std::none_of(chat.history.begin(), chat.history.end(), [message](Ref<GJUserMessage> const& msg) { return message->m_messageID == msg->m_messageID; })) {
                        chat.history.push_back(message);
                        ++chat.unreadCount;
                    }
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
        co_await m_loadMtx.lock();  // don't wanna have loadMessages() change history while downloading chats
        co_await m_downloadMtx.lock();

        log::info("Downloading chats...");
        for (auto& [userID, chat] : m_chats) {
            co_await downloadChat(userID);
        }
        sortChats();
    }

    arc::Future<> ChatHandler::downloadChat(int userID) {
        auto it { m_chats.find(userID) };
        if (it == m_chats.end()) co_return;

        auto gm { GameLevelManager::get() };
        prev_DMD = this;  // just in case
        std::swap(prev_DMD, gm->m_downloadMessageDelegate);

        auto& history { it->second.history };
        for (auto message : history) {
            if (message->m_content.empty()) {
                co_await async::waitForMainThread([gm, message] { gm->downloadUserMessage(message->m_messageID, message->m_outgoing); });
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

    arc::Future<> ChatHandler::sendMessage(int userID, std::string content, std::string subject) {
        co_await m_uploadMtx.lock();

        if (userID == -1) co_return;

        auto gm { GameLevelManager::get() };

        prev_UMD = this;
        std::swap(prev_UMD, gm->m_uploadMessageDelegate);

        auto accountID { gm->accountIDForUserID(userID) };

        co_await async::waitForMainThread([gm, accountID, subject, content] { gm->uploadUserMessage(accountID, subject, content); });
        co_await m_uploadNotif.notified();

        co_await loadMessages();
    }

    void ChatHandler::uploadMessageFinished(int accountID) {
        std::swap(prev_UMD, GameLevelManager::get()->m_uploadMessageDelegate);
        m_uploadNotif.notifyAll();
    }

    void ChatHandler::uploadMessageFailed(int accountID) {
        log::info("Message failed to send");
        std::swap(prev_UMD, GameLevelManager::get()->m_uploadMessageDelegate);
        m_uploadNotif.notifyAll();
    }
}
