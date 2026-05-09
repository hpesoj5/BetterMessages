// TODO: change delegates to http requests to avoid conflict with GD's message delegates

#pragma once

#include <Geode/Geode.hpp>
#include <arc/prelude.hpp>
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
        void refreshChat(int userID);
        arc::Future<> sendMessage(int userID, std::string content, std::string subject = " ");

        arc::Future<> loadMessages();
        bool isLoading() const;

        void saveToDisk();
        void restoreFromDisk();

        int getActiveUserID() const;
        void setActiveUserID(int userID);
        void removeUserID(int userID);

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

        arc::Future<> downloadChat(int userID);
        arc::Future<> downloadChats();

        void downloadMessageFinished(GJUserMessage* message) override;
        void downloadMessageFailed(int id) override;

        void uploadMessageFinished(int accountID) override;
        void uploadMessageFailed(int accountID) override;

        void loadMessagesFinished(CCArray* messages, char const* key) override;
        void loadMessagesFailed(char const* key, GJErrorCode errorType) override;

        std::unordered_map<int, Chat> m_chats;
        Ref<GJUserMessage> m_downloadedMessage;

        MessageListDelegate* prev_MLD { this };
        UploadMessageDelegate* prev_UMD { this };
        DownloadMessageDelegate* prev_DMD { this };

        arc::Notify m_loadNotif;
        arc::Notify m_downloadNotif;
        arc::Notify m_uploadNotif;
        arc::Mutex<int> m_loadMtx {};
        arc::Mutex<int> m_downloadMtx {};
        arc::Mutex<int> m_uploadMtx {};

        std::vector<int> m_activeUserID { };
        int m_highestSentMessageID {};
        int m_highestReceivedMessageID {};
        int m_temporarySentID {};
        int m_temporaryReceivedID {};
        bool m_isLoading {};
        bool m_stopLoading {};
        bool m_isRefreshing {};
    };
}
