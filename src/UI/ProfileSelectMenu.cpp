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
        setAnchorPoint({ 0.5f, 0.5f });

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
        addChild(m_searchInput);

        m_profileScrollLayer = ScrollLayer::create({ contentWidth, contentHeight - m_searchInput->getContentHeight() }, true, true);
        m_profileScrollLayer->m_contentLayer->setLayout(RowLayout::create()->setGap(0.f)->setGrowCrossAxis(true)->setAxisAlignment(AxisAlignment::Start));
        m_profileScrollLayer->setID("ProfileScrollLayer"_spr);
        m_profileScrollLayer->setUserFlag("alk.better-touch-prio/steals-touch");
        m_profileScrollLayer->setStealingTouches(true);
        addChild(m_profileScrollLayer);

        updateLayout();

        m_prevULD = this;
        std::swap(m_prevULD, GameLevelManager::get()->m_userListDelegate);
        GameLevelManager::get()->getUserList(UserListType::Friends);

        return true;
    }

    ProfileSelectMenu::~ProfileSelectMenu() {
        log::info("ProfileSelectMenu instance destroyed");
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

    void ProfileSelectMenu::updateUsers(CCArray* users) {
        // m_profileButtonMenu->updateUsers(users);
        m_profileScrollLayer->m_contentLayer->removeAllChildren();

        auto size { users->count() };
        auto [contentWidth, contentHeight] { m_profileScrollLayer->m_contentLayer->getContentSize() };
        for (auto i { 0uz }; i < size; ++i) {
            auto user { static_cast<GJUserScore*>(users->objectAtIndex(i)) };
            auto button { ProfileButton::create(contentWidth / 4.f, contentHeight / 4.f, user, this, nullptr) };
            button->setID(user->m_userName);
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
}
