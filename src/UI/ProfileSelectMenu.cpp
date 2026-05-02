#include "Constants.hpp"
#include "ProfileSelectMenu.hpp"
#include "ProfileButton.hpp"

namespace BetterMessages {
    Ref<ProfileSelectMenu> ProfileSelectMenu::get() {
        static Ref<ProfileSelectMenu> menu { create() };
        return menu;
    }

    ProfileSelectMenu* ProfileSelectMenu::create() {
        auto ptr { new ProfileSelectMenu };
        if (ptr && ptr->init()) {
            log::info("ProfileSelectMenu instance created");
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    bool ProfileSelectMenu::init() {
        if (!CCMenu::init()) return false;

        setID("profile-select-menu"_spr);

        auto winSize { CCDirector::get()->getWinSize() };
        setContentSize({ winSize.width - 2 * Constants::ChatLayer::PADDING, winSize.height / 2 - Constants::ChatLayer::PADDING });
        setAnchorPoint({ 0.0f, 0.0f });
        setPosition({ Constants::ChatLayer::PADDING, winSize.height / 2.f });

        // search bar
        auto [contentWidth, contentHeight] { getContentSize() };
        m_searchInput = TextInput::create(
            contentWidth,
            "Search Profile",
            "bigFont.fnt"
        );
        m_searchInput->ignoreAnchorPointForPosition(false);
        m_searchInput->setAnchorPoint({ 0.f, 1.0f });
        m_searchInput->setPosition({ 0.f, contentHeight });
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

    ProfileSelectMenu::~ProfileSelectMenu() {
        log::info("ProfileSelectMenu instance destroyed");
    }

    void ProfileSelectMenu::retrieveFriends() {
        m_prevULD = this;
        std::swap(m_prevULD, GameLevelManager::get()->m_userListDelegate);
        GameLevelManager::get()->getUserList(UserListType::Friends);
    }

    void ProfileSelectMenu::updateZOrder() {
        auto zOrder { getZOrder() };
        m_searchInput->setZOrder(zOrder + 1);
        m_profileScrollLayer->setZOrder(zOrder + 1);

        auto size { m_profileScrollLayer->m_contentLayer->getChildrenCount() };
        for (auto i { 0uz }; i < size; ++i) {
            static_cast<ProfileButton*>(m_profileScrollLayer->m_contentLayer->getChildByIndex(i))->updateZOrder(zOrder + 2);
        }

        updateLayout();
    }

    void ProfileSelectMenu::textChanged(CCTextInputNode* input) {
        auto toLower { [](std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; } };

        auto str { input->getString() };

        if (str.empty()) m_filteredUsers = m_users;
        else {
            m_filteredUsers.clear();
            auto size { m_users.size() };
            for (auto i { 0uz }; i < size; ++i) {
                auto user { m_users[i] };
                if (toLower(user->m_userName).find(toLower(str)) != gd::string::npos) {
                    m_filteredUsers.push_back(user);
                }
            }
        }
        displayUsers();
    }

    void ProfileSelectMenu::updateUsers(CCArray* users) {
        auto size { users->count() };
        m_users.clear();
        m_users.reserve(size);

        for (auto i { 0uz }; i < size; ++i) {
            m_users.push_back(static_cast<GJUserScore*>(users->objectAtIndex(i)));
        }

        m_filteredUsers = m_users;
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

    void ProfileSelectMenu::getUserListFinished(CCArray* scores, UserListType type) {
        if (type == UserListType::Friends) {
            std::swap(m_prevULD, GameLevelManager::get()->m_userListDelegate);
            updateUsers(scores);
            log::info("Get friends list request succeeded");
        }
        UserListDelegate::getUserListFinished(scores, type);
    }

    void ProfileSelectMenu::getUserListFailed(UserListType type, GJErrorCode errorType) {
        if (type == UserListType::Friends) {
            std::swap(m_prevULD, GameLevelManager::get()->m_userListDelegate);
            log::info("Get friends list request failed");
        }
        UserListDelegate::getUserListFailed(type, errorType);
    }

    void ProfileSelectMenu::onSelectUser(CCObject* sender) {
        auto button { static_cast<CCMenuItemSpriteExtra*>(sender) };
        log::info("{} selected", button->getID());
    }

    void ProfileSelectMenu::resetInput() {
        m_searchInput->setString("");
        m_searchInput->defocus();
    }
}
