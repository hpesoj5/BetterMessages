#include "Globals.hpp"
#include "ChatHandler.hpp"
#include "ChatLayer.hpp"
#include "ChatMenu.hpp"
#include "Helpers.hpp"
#include "Serialisation.hpp"
#include <Geode/utils/base64.hpp>
#include <arc/time/Sleep.hpp>
#include <algorithm>
#include <utility>

namespace BetterMessages {
    ChatHandler* ChatHandler::get() {
        static ChatHandler handler;
        return &handler;
    }

    ChatHandler::~ChatHandler() { saveToDisk(); }

    void ChatHandler::saveToDisk() {
        auto m { Mod::get() };
        m->setSavedValue("chatData"_spr, m_chats);
        m->setSavedValue("highestReceivedMessageID"_spr, m_highestReceivedMessageID);
        m->setSavedValue("highestSentMessageID"_spr, m_highestSentMessageID);
        // log::info("Successfully saved chat data to disk");
    }

    void ChatHandler::restoreFromDisk() {
        auto m { Mod::get() };
        m_chats = m->getSavedValue<std::unordered_map<int, Chat>>("chatData"_spr);
        m_highestReceivedMessageID = m->getSavedValue<int>("highestReceivedMessageID"_spr);
        m_highestSentMessageID = m->getSavedValue<int>("highestSentMessageID"_spr);
        // log::info("Successfully restored chat data from disk");
    }

    bool ChatHandler::isLoading() const { return m_isLoading || m_isSending; }

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
            auto it { ranges::indexOf(m_activeUserID, userID) };
            if (it) m_activeUserID.erase(m_activeUserID.begin() + it.value());
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

        if (userID != getActiveUserID()) return;
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
        ChatMenu::get()->scrollToBottom();
    }

    void ChatHandler::refreshChat(int userID) {
        auto activeUserID { getActiveUserID() };

        if (userID == -1 || activeUserID != userID) {
            return;
        }

        auto const& history { m_chats[userID].history };
        ChatMenu::get()->displayChatHistory(history);
    }

    std::optional<std::string> ChatHandler::getMessageContentFromString(std::string const& data) {
        // log::info("{}", data);
        if (data.empty() || !string::contains(data, ':')) return {};
        std::vector<std::string> message { string::split(data, ":") };
        for (auto i { 0uz }; i < message.size(); i += 2) {
            int key { numFromString<int>(message[i]).unwrapOr(-1) };
            std::string value { message[i + 1] };
            if (key == 5) {
                auto res { base64::decode(value).ok() };
                if (!res) return {};
                std::vector<std::uint8_t> xorStr { res.value() };
                return xor_cycle(xorStr, "14251");
            }
        }
        return {};
    }

    void ChatHandler::parseMessageString(std::string const& data) {
        // log::info("{}", data);
        if (data.empty() || !string::contains(data, ':')) {
            m_stopLoading = true;
            return;
        }
        std::vector<std::string> parsed { string::split(data, "|") };
        auto size { parsed.size() };
        std::vector<std::vector<std::string>> messages(size);
        std::transform(parsed.begin(), parsed.end(), messages.begin(), [](std::string const& s) { return string::split(s, ":"); });

        for (auto const& messageData : messages) {
            Ref<GJUserMessage> message { GJUserMessage::create() };
            for (auto i { 0uz }; i < messageData.size(); i += 2) {
                int key { numFromString<int>(messageData[i]).unwrapOr(-1) };
                std::string value { messageData[i + 1] };
                // log::info("key: {}, value: {}", key, value);
                switch (key) {
                case 1:
                    message->m_messageID = numFromString<int>(value).unwrapOr(-1);
                    break;

                case 2:
                    message->m_accountID = numFromString<int>(value).unwrapOrDefault();
                    break;

                case 3:
                    message->m_userID = numFromString<int>(value).unwrapOrDefault();
                    break;

                case 4:
                    message->m_title = base64::decodeString(value).unwrapOrDefault();
                    break;

                case 5:
                    message->m_content = base64::decodeString(value).unwrapOrDefault();
                    break;

                case 6:
                    message->m_username = value;
                    break;

                case 7:
                    message->m_uploadDate = value;
                    break;

                case 8:
                    message->m_read = numFromString<int>(value).unwrapOrDefault();
                    break;

                case 9:
                    message->m_outgoing = numFromString<int>(value).unwrapOrDefault();
                    break;

                default:
                    break;
                }
            }
            if (message->m_messageID != -1 && Globals::Accounts::accountIDs.find(message->m_userID) != Globals::Accounts::accountIDs.end()) {
                // log::info("messageID: {}, user: {}, title: {}, content: {}, outgoing: {}", message->m_messageID, message->m_username, message->m_title, message->m_content, message->m_outgoing);
                auto& history { m_chats[message->m_userID].history };
                if (message->m_outgoing) {
                    if (message->m_messageID <= m_highestSentMessageID)  {
                        m_stopLoading = true;
                        continue;
                    }
                    history.push_back(message);
                    m_temporarySentID = std::max(m_temporarySentID, message->m_messageID);
                }
                else {
                    if (message->m_messageID <= m_highestReceivedMessageID) {
                        m_stopLoading = true;
                        continue;
                    }
                    history.push_back(message);
                    m_temporaryReceivedID = std::max(m_temporaryReceivedID, message->m_messageID);
                }
            }
        }
    }

    void ChatHandler::loadMessages() {
        if (m_isLoading) return;
        auto accountID { GJAccountManager::get()->m_accountID };
        std::string gjp2 { GJAccountManager::get()->m_GJP2 };
        if (accountID <= 0 || gjp2.empty()) return;
        // log::info("loading messages...");
        m_isLoading = true;
        m_stopLoading = false;
        ChatMenu::get()->setLoadingSpinner(true);
        async::spawn([this, accountID, gjp2] -> arc::Future<> {
            int page {};
            int retryCount {};
            while (true) {
                web::WebRequest req {};
                req.bodyString(fmt::format(
                    "accountID={}&gjp2={}&secret={}&page={}&getSent=0",
                    accountID,
                    gjp2,
                    Globals::Requests::SOCIAL_SECRET,
                    page
                ));
                req.userAgent("");
                req.header("Content-Type", "application/x-www-form-urlencoded");

                web::WebResponse res { co_await sendRequest(req, "https://www.boomlings.com/database/getGJMessages20.php") };

                if (res.ok() && res.string().isOk()) {
                    std::string str { res.string().unwrap() };
                    // log::info("page: {}, response: {}", page, str);
                    co_await async::waitForMainThread([this, &str] { parseMessageString(str); });
                    // log::info("Page {} of received messages loaded successfully", page);
                    if (m_stopLoading) break;
                    ++page;
                }
                else {
                    log::info("Get received messages page {} request failed: {}", page, res.code());
                    if (++retryCount >= 5) break;
                }
            }

            m_highestReceivedMessageID = std::max(m_highestReceivedMessageID, m_temporaryReceivedID);
            m_temporaryReceivedID = 0;

            page = 0;
            retryCount = 0;
            m_stopLoading = false;
            while (true) {
                web::WebRequest req {};
                req.bodyString(fmt::format(
                    "accountID={}&gjp2={}&secret={}&page={}&total=50&getSent=1",
                    accountID,
                    gjp2,
                    Globals::Requests::SOCIAL_SECRET,
                    page
                ));
                req.userAgent("");
                req.header("Content-Type", "application/x-www-form-urlencoded");

                web::WebResponse res { co_await sendRequest(req, "https://www.boomlings.com/database/getGJMessages20.php") };

                if (res.ok() && res.string().isOk()) {
                    std::string str { res.string().unwrap() };
                    // log::info("page: {}, response: {}", page, str);
                    co_await async::waitForMainThread([this, &str] { parseMessageString(str); });
                    // log::info("Page {} of sent messages loaded successfully", page);
                    if (m_stopLoading) break;
                    ++page;
                }
                else {
                    log::info("Get sent messages page {} request failed: {}", page, res.code());
                    if (++retryCount >= 5) break;
                }
            }

            m_highestSentMessageID = std::max(m_highestSentMessageID, m_temporarySentID);
            m_temporarySentID = 0;

            // log::info("Messages loaded successfully");
        }, [this] {
            sortChats();
            // log::info("Chats sorted");
            downloadChats();
        }).setName("LoadMessages"_spr);
    }

    void ChatHandler::sortChats() {
        for (auto& [userID, chat] : m_chats) {
            std::sort(chat.history.begin(), chat.history.end(), [](Ref<GJUserMessage> const& a, Ref<GJUserMessage> const& b) { return a->m_messageID < b->m_messageID; });
        }
    }

    void ChatHandler::downloadChats() {
        int accountID { GJAccountManager::get()->m_accountID };
        std::string gjp2 { GJAccountManager::get()->m_GJP2 };
        if (accountID <= 0 || gjp2.empty()) return;
        // log::info("Downloading message content...");
        std::vector<int> userIDs;
        userIDs.reserve(m_chats.size());
        for (auto& [userID, _] : m_chats) userIDs.push_back(userID);
        async::spawn([this, accountID, gjp2, userIDs] -> arc::Future<> {
            for (auto userID : userIDs) co_await downloadChat(userID, accountID, gjp2);
        }, [this] {
            // log::info("All messages downloaded");
            m_isLoading = false;
            if (ChatLayer::get()->isOpen()) refreshChat(getActiveUserID());
            ChatMenu::get()->setLoadingSpinner(false);
        }).setName("DownloadChats"_spr);
    }

    arc::Future<> ChatHandler::downloadChat(int userID, int accountID, std::string const& gjp2) {
        auto size { (co_await async::waitForMainThread<size_t>([this, userID] { return m_chats[userID].history.size(); })).value_or(0) };
        for (int i { static_cast<int>(size) - 1 }; i >= 0; --i) {
            // log::info("{}, {}", i, size);
            int messageID { -1 };
            bool sent {};
            co_await async::waitForMainThread([this, i, userID, &messageID, &sent] {
                auto& message { m_chats[userID].history[i] };
                if (message->m_content.empty()) {
                    messageID = message->m_messageID;
                    sent = message->m_outgoing;
                }
            });
            if (messageID != -1) {
                web::WebRequest req {};
                req.bodyString(fmt::format(
                    "accountID={}&gjp2={}&messageID={}&secret={}&isSender={}",
                    accountID,
                    gjp2,
                    messageID,
                    Globals::Requests::SOCIAL_SECRET,
                    static_cast<int>(sent)
                ));
                req.userAgent("");
                req.header("Content-Type", "application/x-www-form-urlencoded");

                web::WebResponse res { co_await sendRequest(req, "https://www.boomlings.com/database/downloadGJMessage20.php") };

                if (res.ok() && res.string().isOk()) {
                    std::string str { res.string().unwrap() };
                    // log::info("Download request (userID: {}, messageID: {}, sent: {}) response: {}", userID, messageID, sent, str);
                    co_await async::waitForMainThread([this, i, userID, &str] {
                        auto content { getMessageContentFromString(str) };
                        if (content) {
                            // log::info("message content: {}", content.value());
                            m_chats[userID].history[i]->m_content = content.value();
                        }
                    });
                }
                else {
                    log::info("Download message {} request failed: {}", messageID, res.code());
                }
            }
        }
    }

    void ChatHandler::sendMessage(int toAccountID, std::string const& content, std::string const& subject) {
        m_sentMessageQueue.emplace(toAccountID, content, subject);
        int accountID { GJAccountManager::get()->m_accountID };
        std::string gjp2 { GJAccountManager::get()->m_GJP2 };
        if (m_isSending || accountID <= 0 || gjp2.empty()) return;
        m_isSending = true;
        ChatMenu::get()->setLoadingSpinner(true);
        async::spawn([this, accountID, gjp2] -> arc::Future<> {
            while (true) {
                int toAccountID { -1 };
                std::string content;
                std::string subject;
                co_await async::waitForMainThread([this, &toAccountID, &content, &subject] {
                    if (!m_sentMessageQueue.empty()) {
                        auto [id, c, s] { m_sentMessageQueue.front() };
                        m_sentMessageQueue.pop();
                        toAccountID = id;
                        content = std::move(c);
                        subject = std::move(s);
                    }
                });
                if (toAccountID == -1) break;
                // log::info("accountID: {}, content: {}, subject: {}", toAccountID, content, subject);
                web::WebRequest req {};
                req.bodyString(fmt::format(
                    "accountID={}&gjp2={}&toAccountID={}&subject={}&body={}&secret={}",
                    accountID,
                    gjp2,
                    toAccountID,
                    base64::encode(subject),
                    base64::encode(xor_cycle(content, "14251")),
                    Globals::Requests::SOCIAL_SECRET
                ));
                req.userAgent("");
                req.header("Content-Type", "application/x-www-form-urlencoded");

                web::WebResponse res { co_await sendRequest(req, "https://www.boomlings.com/database/uploadGJMessage20.php") };

                if (res.ok() && res.string().isOk()) {
                    std::string str { res.string().unwrap() };
                    if (str == "-1") log::info("a problem occurred while sending the message to account {} with content {} (code {})", toAccountID, content, res.code());
                }
                else {
                    log::info("message sent to account {} with content {} failed: {}", toAccountID, content, res.code());
                }
            }
        }, [this] {
            m_isSending = false;
            loadMessages();
        }).setName("SendMessage"_spr);
    }
}
