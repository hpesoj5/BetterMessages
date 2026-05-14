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

        // async::spawn(downloadChat(userID), [this, userID] {
            auto const& history { m_chats[userID].history };  // if element does not exist, create it
            auto const& input { m_chats[userID].draftMessage };
            ChatMenu::get()->setInputString(input);
            ChatMenu::get()->displayChatHistory(history);
        // });
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

    arc::Future<> ChatHandler::loadMessages() {
        if (!m_loadMtx.tryLock()) co_return;

        co_await async::waitForMainThread([this] {
            log::debug("Fetching messages...");
            auto gm { GameLevelManager::get() };
            ChatMenu::get()->setLoading(true);
            ChatMenu::get()->setLoadingSpinner(true);
            prev_MLD = this;
            std::swap(prev_MLD, gm->m_messageListDelegate);
            prev_DMD = this;
            std::swap(prev_DMD, gm->m_downloadMessageDelegate);
        });

        // unsent messages
        int page {};
        while (true) {
            co_await async::waitForMainThread([page] { GameLevelManager::get()->getUserMessages(false, page, 50); });
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
            co_await async::waitForMainThread([page] { GameLevelManager::get()->getUserMessages(true, page, 50); });
            co_await m_loadNotif.notified();
            if (m_stopLoading) {
                m_stopLoading = false;
                break;
            }
            ++page;
        }

        co_await async::waitForMainThread([this] {
            auto gm { GameLevelManager::get() };
            std::swap(prev_MLD, gm->m_messageListDelegate);
            std::swap(prev_DMD, gm->m_downloadMessageDelegate);
        });

        if (m_temporaryReceivedID > m_highestReceivedMessageID) m_highestReceivedMessageID = m_temporaryReceivedID;
        if (m_temporarySentID > m_highestSentMessageID) m_highestSentMessageID = m_temporarySentID;

        m_temporaryReceivedID = 0;
        m_temporarySentID = 0;

        co_await async::waitForMainThread([this] {
            sortChats();
            log::info("Chats fetched");
            ChatMenu::get()->setLoading(false);
            ChatMenu::get()->setLoadingSpinner(false);
        });
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
        m_loadNotif.notifyOne();
    }

    void ChatHandler::loadMessagesFailed(char const* key, GJErrorCode errorType) {
        // -2: empty page
        if (static_cast<int>(errorType) == -2) m_stopLoading = true;
        m_loadNotif.notifyOne();
    }

    arc::Future<> ChatHandler::downloadChats() {
        log::debug("Downloading chats...");
        for (auto& [userID, chat] : m_chats) {
            co_await downloadChat(userID);
        }
    }

    arc::Future<> ChatHandler::downloadChat(int userID) {
        co_await m_loadMtx.lock();
        auto it { m_chats.find(userID) };
        if (it == m_chats.end()) co_return;

        co_await async::waitForMainThread([this] {
            auto gm { GameLevelManager::get() };
            prev_DMD = this;  // just in case
            std::swap(prev_DMD, gm->m_downloadMessageDelegate);
        });

        auto& history { it->second.history };
        auto size { history.size() };
        for (auto i { 0uz }; i < size; ++i) {
            m_messageSuccessfullyFetched = false;
            bool skip {};
            co_await async::waitForMainThread([i, size, &history, &skip] {
                auto const& message { history[i] };
                if (message->m_content.empty()) {
                    log::info("i: {}, size: {}, messageID: {}, username: {}, content: {}", i, size, message->m_messageID, message->m_username, message->m_content);
                    GameLevelManager::get()->downloadUserMessage(message->m_messageID, message->m_outgoing);
                }
                else skip = true;
            });
            if (skip) continue;
            co_await m_downloadNotif.notified();
            if (m_messageSuccessfullyFetched) {
                co_await async::waitForMainThread([this, i, &history] {
                    // log::info("message {} successfully fetched", i);
                    auto& message { history[i] };
                    message->m_content = m_downloadedMessage->m_content;
                });
            }
        }

        co_await async::waitForMainThread([this, userID] {
            std::swap(prev_DMD, GameLevelManager::get()->m_downloadMessageDelegate);
            log::info("All messages from {} downloaded", userID);
        });
    }

    void ChatHandler::downloadMessageFinished(GJUserMessage* message) {
        m_downloadedMessage = message;
        m_messageSuccessfullyFetched = true;
        m_downloadNotif.notifyOne();
    }

    void ChatHandler::downloadMessageFailed(int id) {
        log::info("Could not download message {}", id);
        m_downloadedMessage = nullptr;
        m_messageSuccessfullyFetched = false;
        m_downloadNotif.notifyOne();
    }

    void ChatHandler::sortChats() {
        for (auto& [userID, chat] : m_chats) {
            std::sort(chat.history.begin(), chat.history.end(), [](Ref<GJUserMessage> const& a, Ref<GJUserMessage> const& b) { return a->m_messageID < b->m_messageID; });
        }
    }

    arc::Future<> ChatHandler::sendMessage(int userID, std::string content, std::string subject) {
        co_await m_uploadMtx.lock();

        if (userID == -1) co_return;

        co_await async::waitForMainThread([this, userID, content, subject] {
            auto gm { GameLevelManager::get() };

            prev_UMD = this;
            std::swap(prev_UMD, gm->m_uploadMessageDelegate);

            auto accountID { gm->accountIDForUserID(userID) };

            gm->uploadUserMessage(accountID, subject, content);
        });

        co_await m_uploadNotif.notified();

        co_await loadMessages();
        if (userID == getActiveUserID()) {
            co_await async::waitForMainThread([this, userID] { refreshChat(userID); });
        }
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
