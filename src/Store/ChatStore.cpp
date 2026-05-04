#include "ChatStore.hpp"

namespace BetterMessages {
    ChatStore* ChatStore::get() {
        static ChatStore chatStore;
        return &chatStore;
    }

    void ChatStore::sendMessage(int userID, gd::string content, gd::string subject) {
        // only allow one request at a time (stop them from spamming)
        if (prev_UMD != this) {
            log::info("Message sent to {} was rate limited.", userID);
            return;
        }
        auto gm { GameLevelManager::get() };
        int accountID = gm->accountIDForUserID(userID);
        prev_UMD = this;
        std::swap(prev_UMD, gm->m_uploadMessageDelegate);
        gm->uploadUserMessage(accountID, subject, content);
    }

    void ChatStore::setActiveConversation(int userID) { m_activeUserID = userID; }

    Conversation ChatStore::getConversation(int userID) const {
        auto conversation { m_conversations.find(userID) };
        if (conversation == m_conversations.end()) return {};
        else return conversation->second;
    }

    Conversation ChatStore::getActiveConversation() const { return getConversation(m_activeUserID); }

    void ChatStore::uploadMessageFinished(int accountID) {
        auto gm { GameLevelManager::get() };
        std::swap(prev_UMD, gm->m_uploadMessageDelegate);
        int userID { gm->userIDForAccountID(accountID) };
        log::info("Message successfully sent to userID {}.", userID);
        UploadMessageDelegate::uploadMessageFinished(accountID);
    }

    void ChatStore::uploadMessageFailed(int accountID) {
        auto gm { GameLevelManager::get() };
        std::swap(prev_UMD, gm->m_uploadMessageDelegate);
        int userID { gm->userIDForAccountID(accountID) };
        log::info("Message to userID {} failed.", userID);
        UploadMessageDelegate::uploadMessageFailed(accountID);
    }

    void ChatStore::loadMessagesFinished(CCArray* messages, char const* key) {
        // std::swap(prev_MLD, GameLevelManager::get()->m_messageListDelegate);

        // auto size { messages->count() };
        // for (int i { 0uz }; i < size; ++i) {
        //     auto msg { static_cast<GJUserMessage*>(messages->objectAtIndex(i)) };

        //     auto userID { msg->m_userID };

        //     auto it { m_conversations.find(userID) };

        //     if (it == m_conversations.end()) m_conversations.emplace(userID, Conversation{ { msg }, msg->m_outgoing ? 0 : 1 });
        //     else {
        //         auto& conversation { it->second };
        //         auto& history { conversation.history };

        //         bool exists { std::any_of(history.begin(), history.end(), [msg](Ref<GJUserMessage>& m) { return msg->m_messageID == m->m_messageID; }) };

        //         if (!exists) {
        //             history.emplace_back(msg);
        //             if (msg->m_outgoing) ++conversation.unreadCount;
        //         }
        //     }
        // }

        // for (auto& [userID, conversation] : m_conversations) {
        //     std::sort(conversation.history.begin(), conversation.history.end(), [](Ref<GJUserMessage> const& a, Ref<GJUserMessage> const& b) { return a->m_uploadDate < b->m_uploadDate; });
        // }

        // notifyListeners(m_activeUserID);
        MessageListDelegate::loadMessagesFinished(messages, key);
    }

    void ChatStore::loadMessagesFailed(char const* key, GJErrorCode errorType) {
        // std::swap(prev_MLD, GameLevelManager::get()->m_messageListDelegate);
        // log::info("{}: Failed to load messages. Error: {}", key, static_cast<int>(errorType));
        MessageListDelegate::loadMessagesFailed(key, errorType);
    }
}
