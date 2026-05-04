// TODO: find a way of writing chat messages to save data
// TODO: write a task to retrieve online messages wtih other users
// TODO: verify sendMessage works, and have deleteMessage (only accessible from messeage list popup) reflect in chat history
// TODO: actual chat ui with textinput for writing messages with default subject

#pragma once

#include <Geode/Geode.hpp>
#include <unordered_map>
#include <vector>

using namespace geode::prelude;

namespace BetterMessages {

    struct Conversation {
        std::vector<Ref<GJUserMessage>> history;
        int unreadCount;
    };

    // uhh a bit overkill for just a single listener
    class ChatStoreListener {
    public:
        virtual void onMessagesUpdated(int userID) = 0;
    };

    class ChatStore final : public MessageListDelegate, DownloadMessageDelegate, UploadMessageDelegate {
    public:
        static ChatStore* get();

        void sendMessage(int userID, gd::string content, gd::string subject = " ");
        void setActiveConversation(int userID);
        Conversation getConversation(int userID) const;
        Conversation getActiveConversation() const;

        void notifyListeners(int userID) {
            for (ChatStoreListener* l : m_listeners) l->onMessagesUpdated(userID);
        }

    private:
        ChatStore() = default;
        ~ChatStore() = default;

        ChatStore(ChatStore const& other) = delete;
        ChatStore(ChatStore&& other) = delete;
        ChatStore& operator=(ChatStore const& other) = delete;
        ChatStore& operator=(ChatStore&& other) = delete;

        void uploadMessageFinished(int accountID) override;
        void uploadMessageFailed(int accountID) override;

        // have a arc::Task to continuously load messages both sent and received until an existing message is reached
        void loadMessagesFinished(CCArray* messages, char const* key) override;
        void loadMessagesFailed(char const* key, GJErrorCode errorType) override;

        std::unordered_map<int, Conversation> m_conversations;

        std::vector<ChatStoreListener*> m_listeners;

        MessageListDelegate* prev_MLD { this };
        UploadMessageDelegate* prev_UMD { this };
        DownloadMessageDelegate* prev_DMD { this };

        int m_activeUserID;
    };
}
