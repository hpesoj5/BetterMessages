#include "Globals.hpp"
#include "ChatMenu.hpp"
#include "Helpers.hpp"
#include "ProfileSelectMenu.hpp"
#include "ProfileButton.hpp"
#include <Geode/utils/string.hpp>
#include <arc/time/Sleep.hpp>
#include <algorithm>

namespace BetterMessages {
    Ref<ProfileSelectMenu> ProfileSelectMenu::get() {
        static Ref<ProfileSelectMenu> menu { create() };
        return menu;
    }

    ProfileSelectMenu* ProfileSelectMenu::create() {
        auto ptr { new ProfileSelectMenu };
        if (ptr && ptr->init()) {
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    bool ProfileSelectMenu::init() {
        if (!CCMenu::init()) return false;

        setID("ProfileSelectMenu"_spr);

        auto winSize { CCDirector::get()->getWinSize() };
        setContentSize({ winSize.width - 2 * Globals::ChatLayer::PADDING, winSize.height / 2 - Globals::ChatLayer::PADDING });
        setAnchorPoint({ 0.0f, 0.0f });
        setPosition(Globals::ChatLayer::PADDING, winSize.height / 2.f);

        // search bar
        auto [contentWidth, contentHeight] { getContentSize() };
        m_searchInput = TextInput::create(
            contentWidth,
            "Search Profile",
            "bigFont.fnt"
        );
        m_searchInput->ignoreAnchorPointForPosition(false);
        m_searchInput->setAnchorPoint({ 0.f, 1.0f });
        m_searchInput->setPosition(0.f, contentHeight);
        m_searchInput->setTextAlign(TextInputAlign::Left);
        m_searchInput->setID("ProfileSearchInput"_spr);
        m_searchInput->setDelegate(this);
        addChild(m_searchInput);

        m_profileScrollLayer = ScrollLayer::create({ contentWidth, contentHeight - m_searchInput->getContentHeight() }, true, true);
        m_profileScrollLayer->m_contentLayer->setLayout(RowLayout::create()->setGap(0.f)->setGrowCrossAxis(true)->setAxisAlignment(AxisAlignment::Start));
        m_profileScrollLayer->setID("ProfileScrollLayer"_spr);
        m_profileScrollLayer->setUserFlag("alk.better-touch-prio/steals-touch");
        m_profileScrollLayer->setStealingTouches(true);
        addChild(m_profileScrollLayer);

        updateLayout();

        retrieveFriends();

        return true;
    }

    ProfileSelectMenu::~ProfileSelectMenu() {}

    void ProfileSelectMenu::enableScrollWheel(bool enabled) {
        m_profileScrollLayer->enableScrollWheel(enabled);
        log::debug("ProfileScrollLayer scrollWheel {}", enabled ? "enabled" : "disabled");
    }

    void ProfileSelectMenu::updateZOrder(int zOrder) {
        setZOrder(zOrder);

        m_searchInput->setZOrder(zOrder + 1);
        m_profileScrollLayer->setZOrder(zOrder + 1);

        auto size { m_profileScrollLayer->m_contentLayer->getChildrenCount() };
        for (auto i { 0uz }; i < size; ++i) {
            static_cast<ProfileButton*>(m_profileScrollLayer->m_contentLayer->getChildByIndex(i))->updateZOrder(zOrder + 2);
        }

        updateLayout();
    }

    void ProfileSelectMenu::retrieveFriends() {
        if (m_isLoading) return;
        m_isLoading = true;
        auto accountID { GJAccountManager::get()->m_accountID };
        std::string gjp2 { GJAccountManager::get()->m_GJP2 };
        if (accountID <= 0 || gjp2.empty()) return;
        async::spawn([this, accountID, gjp2] -> arc::Future<> {
            web::WebRequest req {};
            req.bodyString(fmt::format(
            "accountID={}&gjp2={}&secret={}&type={}",
                accountID,
                gjp2,
                Globals::Requests::SOCIAL_SECRET,
                0
            ));
            req.userAgent("");
            req.header("Content-Type", "application/x-www-form-urlencoded");

            co_await arc::sleep((co_await async::waitForMainThread<asp::Duration>([] { return timeToNextRequest(); })).value_or({}));
            auto res { co_await req.post("https://www.boomlings.com/database/getGJUserList20.php") };
            co_await async::waitForMainThread([] { setLastRequestTime(); });

            if (res.ok() && res.string().isOk()) {
                co_await async::waitForMainThread([this, res] {
                    parseFriendString(res.string().unwrap());
                    displayUsers();
                });
            }
            else log::debug("Get friend list request failed: {}", res.code());
        }, [this] { m_isLoading = false; });
    }

    void ProfileSelectMenu::parseFriendString(std::string const& data) {
        std::vector<std::string> parsed { string::split(data, "|") };
        // log::info("parsed: {}", parsed);
        auto size { parsed.size() };
        std::vector<std::vector<std::string>> friends(size);
        std::transform(parsed.begin(), parsed.end(), friends.begin(), [](std::string const& s) { return string::split(s, ":"); });
        // log::info("friends: {}", friends);

        m_users.clear();
        m_users.reserve(size);
        for (auto const& data : friends) {
            Ref<GJUserScore> user { GJUserScore::create() };
            for (auto i { 0uz }; i < data.size(); i += 2) {
                int key { numFromString<int>(data[i]).ok().value() };
                std::string value { data[i + 1] };
                // log::info("key: {}, value: {}", key, value);
                switch (key) {
                case 1:
                    user->m_userName = value;
                    break;

                case 2:
                    user->m_userID = numFromString<int>(value).unwrapOr(-1);
                    break;

                case 9:
                    user->m_iconID = numFromString<int>(value).unwrapOr(-1);
                    break;

                case 10:
                    user->m_color1 = numFromString<int>(value).unwrapOr(-1);
                    break;

                case 11:
                    user->m_color2 = numFromString<int>(value).unwrapOr(-1);
                    break;

                case 14:
                    user->m_iconType = static_cast<IconType>(numFromString<int>(value).unwrapOrDefault());
                    break;

                case 15:
                    user->m_special = numFromString<int>(value).unwrapOr(-1);
                    break;

                case 16:
                    user->m_accountID = numFromString<int>(value).unwrapOr(-1);
                    break;

                case 18:
                    user->m_messageState = numFromString<int>(value).unwrapOr(-1);
                    break;

                case 41:
                    break;

                case 51:
                    user->m_color3 = numFromString<int>(value).unwrapOr(-1);
                    break;

                default:
                    break;
                }
            }
            if (user->m_userID != -1) {
                setAccountIDForUserID(user->m_userID, user->m_accountID);
                m_users.push_back(user);
            }
        }
        m_filteredUsers = m_users;
    }

    void ProfileSelectMenu::textChanged(CCTextInputNode* input) {
        std::string str { input->getString() };

        if (str.empty()) m_filteredUsers = m_users;
        else {
            m_filteredUsers.clear();
            auto size { m_users.size() };
            for (auto i { 0uz }; i < size; ++i) {
                auto user { m_users[i] };
                if (string::toLower(user->m_userName).find(string::toLower(str)) != std::string::npos) {
                    m_filteredUsers.push_back(user);
                }
            }
        }
        displayUsers();
    }

    void ProfileSelectMenu::displayUsers() {
        m_profileScrollLayer->m_contentLayer->removeAllChildren();

        auto size { m_filteredUsers.size() };
        auto [contentWidth, contentHeight] { m_profileScrollLayer->getContentSize() };
        for (auto i { 0uz }; i < size; ++i) {
            auto user { m_filteredUsers[i] };
            auto button { ProfileButton::create(contentWidth / 4.f, contentHeight / 4.f, user, this, menu_selector(ProfileSelectMenu::onSelectUser)) };
            m_profileScrollLayer->m_contentLayer->addChild(button);
        }

        m_profileScrollLayer->m_contentLayer->updateLayout();
        m_profileScrollLayer->scrollToTop();
    }

    void ProfileSelectMenu::onSelectUser(CCObject* sender) {
        auto button { static_cast<CCMenuItemSpriteExtra*>(sender) };
        auto user { static_cast<GJUserScore*>(button->getUserObject()) };

        ChatMenu::get()->goToUser(user);
    }

    void ProfileSelectMenu::resetInput() {
        m_searchInput->setString("");
        m_searchInput->defocus();
    }
}
