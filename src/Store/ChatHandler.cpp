#include "Constants.hpp"
#include "ChatHandler.hpp"
#include "ChatMenu.hpp"
#include "Serialisation.hpp"
#include <Geode/utils/base64.hpp>
#include <algorithm>
#include <utility>

namespace BetterMessages {
    ChatHandler* ChatHandler::get() {
        static ChatHandler handler;
        return &handler;
    }

    ChatHandler::~ChatHandler() { saveToDisk(); }

    void ChatHandler::saveToDisk() {
        if (m_mtx.tryLock()) {
            auto m { Mod::get() };
            m->setSavedValue("chatData", m_chats);
            m->setSavedValue("highestReceivedMessageID", m_highestReceivedMessageID);
            m->setSavedValue("highestSentMessageID", m_highestSentMessageID);
            log::info("Successfully saved to disk");
        }
    }

    void ChatHandler::restoreFromDisk() {
        async::spawn([this] -> arc::Future<> {
            co_await m_mtx.lock();
            co_await async::waitForMainThread([this] {
                auto m { Mod::get() };
                m_chats = m->getSavedValue<std::unordered_map<int, Chat>>("chatData");
                m_highestReceivedMessageID = m->getSavedValue<int>("highestReceivedMessageID");
                m_highestSentMessageID = m->getSavedValue<int>("highestSentMessageID");
                log::info("Successfully restored from disk");
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

        async::spawn([this, userID] -> arc::Future<> {
            co_await m_mtx.lock();
            co_await async::waitForMainThread([this, userID] {
                auto const& history { m_chats[userID].history };  // if element does not exist, create it
                auto const& input { m_chats[userID].draftMessage };
                ChatMenu::get()->setInputString(input);
                ChatMenu::get()->displayChatHistory(history);
            });
        });
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
        auto activeUserID { getActiveUserID() };

        if (userID == -1 || activeUserID != userID) {
            return;
        }

        async::spawn([this, userID] -> arc::Future<> {
            co_await m_mtx.lock();
            co_await async::waitForMainThread([this, userID] {
                auto const& history { m_chats[userID].history };
                ChatMenu::get()->displayChatHistory(history);
            });
        });
    }

    void ChatHandler::parseMessageString(std::string const& data) {
        std::vector<std::string> parsed { string::split(data, "|") };
        auto size { parsed.size() };
        std::vector<std::vector<std::string>> messages(size);
        std::transform(parsed.begin(), parsed.end(), messages.begin(), [](std::string const& s) { return string::split(s, ":"); });

        for (auto const& data : messages) {
            Ref<GJUserMessage> message { GJUserMessage::create() };
            for (auto i { 0uz }; i < data.size(); i += 2) {
                int key { numFromString<int>(data[i]).ok().value() };
                std::string value { data[i + 1] };
                // log::info("key: {}, value: {}", key, value);
                switch (key) {
                case 1:
                    message->m_messageID = numFromString<int>(value).ok().value();
                    break;

                case 2:
                    message->m_accountID = numFromString<int>(value).ok().value();
                    break;

                case 3:
                    message->m_userID = numFromString<int>(value).ok().value();
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
            if (message->m_messageID != -1) {
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
        m_isLoading = true;
        m_stopLoading = false;
        auto accountID { GJAccountManager::get()->m_accountID };
        std::string gjp2 { GJAccountManager::get()->m_GJP2 };
        if (accountID <= 0 || gjp2.empty()) return;
        async::spawn([this, accountID, gjp2] -> arc::Future<> {
            int page {};
            co_await m_mtx.lock();
            while (true) {
                web::WebRequest req {};
                req.bodyString(fmt::format(
                    "accountID={}&gjp2={}&secret={}&page={}&getSent=0",
                    accountID,
                    gjp2,
                    Constants::Requests::SOCIAL_SECRET,
                    page
                ));
                req.userAgent("");
                req.header("Content-Type", "application/x-www-form-urlencoded");

                auto res { co_await req.post("https://www.boomlings.com/database/getGJMessages20.php") };
                if (res.ok() && res.string().isOk()) {
                    auto str { res.string().unwrap() };
                    // log::info("page: {}, response: {}", page, str);
                    if (str == "-2") break;
                    co_await async::waitForMainThread([this, &str] {
                        parseMessageString(str);
                    });
                    log::info("Page {} of received messages loaded successfully", page);
                    if (m_stopLoading) break;
                    ++page;
                }
                else {
                    log::debug("Get received messages page {} request failed: {}", page, res.code());
                    break;
                }
            }

            m_highestReceivedMessageID = std::max(m_highestReceivedMessageID, m_temporaryReceivedID);
            m_temporaryReceivedID = 0;

            page = 0;
            while (true) {
                web::WebRequest req {};
                req.bodyString(fmt::format(
                    "accountID={}&gjp2={}&secret={}&page={}&total=50&getSent=1",
                    accountID,
                    gjp2,
                    Constants::Requests::SOCIAL_SECRET,
                    page
                ));
                req.userAgent("");
                req.header("Content-Type", "application/x-www-form-urlencoded");

                auto res { co_await req.post("https://www.boomlings.com/database/getGJMessages20.php") };
                if (res.ok() && res.string().isOk()) {
                    auto str { res.string().unwrap() };
                    // log::info("page: {}, response: {}", page, str);
                    if (str == "-2") break;
                    co_await async::waitForMainThread([this, &str] {
                        parseMessageString(str);
                    });
                    log::info("Page {} of sent messages loaded successfully", page);
                    if (m_stopLoading) break;
                    ++page;
                }
                else {
                    log::debug("Get sent messages page {} request failed: {}", page, res.code());
                    break;
                }
            }

            m_highestSentMessageID = std::max(m_highestSentMessageID, m_temporarySentID);
            m_temporarySentID = 0;

        }, [this] {
            log::info("Messages loaded successfully");
            m_isLoading = false;
            sortChats();
            log::info("Messages sorted");
        });
    }

    void ChatHandler::sortChats() {
        async::spawn([this] -> arc::Future<> {
            co_await m_mtx.lock();
            co_await async::waitForMainThread([this] {
                for (auto& [userID, chat] : m_chats) {
                    std::sort(chat.history.begin(), chat.history.end(), [](Ref<GJUserMessage> const& a, Ref<GJUserMessage> const& b) { return a->m_messageID < b->m_messageID; });
                }
            });
        });
    }
}
