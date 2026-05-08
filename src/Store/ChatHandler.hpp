// TODO: find a way of writing chat messages to save data
// TODO: write a task to retrieve online messages wtih other users
// TODO: verify sendMessage works, and have deleteMessage (only accessible from messeage list popup) reflect in chat history
// TODO: actual chat ui with textinput for writing messages with default subject

#pragma once

#include <Geode/Geode.hpp>
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

    class ChatHandler final : MessageListDelegate, DownloadMessageDelegate, UploadMessageDelegate {
    public:
        static ChatHandler* get();

        void switchChat(int userID);
        void sendMessage(std::string content, std::string subject = " ");
        arc::Future<> loadMessages();

        bool isLoading() const;

    private:
        ChatHandler() = default;
        ~ChatHandler() = default;

        ChatHandler(ChatHandler const& other) = delete;
        ChatHandler(ChatHandler&& other) = delete;
        ChatHandler& operator=(ChatHandler const& other) = delete;
        ChatHandler& operator=(ChatHandler&& other) = delete;

        void saveChat(int userID);
        void restoreChat(int userID);
        void sortChats();

        void downloadChat(int userID);
        void downloadMessages();

        void downloadMessageFinished(GJUserMessage* message) override;
        void downloadMessageFailed(int id) override;

        void uploadMessageFinished(int accountID) override;
        void uploadMessageFailed(int accountID) override;

        void loadMessagesFinished(CCArray* messages, char const* key) override;
        void loadMessagesFailed(char const* key, GJErrorCode errorType) override;

        std::unordered_map<int, Chat> m_chats;
        std::vector<std::pair<Ref<GJUserMessage>, size_t>> m_messagesToDownload;

        MessageListDelegate* prev_MLD { this };
        UploadMessageDelegate* prev_UMD { this };
        DownloadMessageDelegate* prev_DMD { this };

        arc::Notify m_notif;

        int m_messageIndex {};
        int m_activeUserID { -1 };
        int m_highestSentMessageID {};
        int m_highestReceivedMessageID {};
        int m_temporarySentID {};
        int m_temporaryReceivedID {};
        bool m_isLoading {};
        bool m_stopLoading {};
    };
}
