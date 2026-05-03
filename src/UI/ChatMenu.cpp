#include "Constants.hpp"
#include "ChatMenu.hpp"
#include "TabButton.hpp"

namespace BetterMessages {
    Ref<ChatMenu> ChatMenu::get() {
        static Ref<ChatMenu> menu { create() };
        return menu;
    }

    ChatMenu* ChatMenu::create() {
        auto ptr { new ChatMenu };
        if (ptr && ptr->init()) {
            log::info("ChatMenu instance created");
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    bool ChatMenu::init() {
        if (!CCMenu::init()) return false;

        setID("chat-menu"_spr);

        auto winSize { CCDirector::get()->getWinSize() };

        setContentSize({ winSize.width, winSize.height / 2.f });
        setAnchorPoint({ 0.f, 0.f });
        setPosition({ 0.f, 0.f });

        auto [contentWidth, contentHeight] { getContentSize() };
        m_tabButtonMenu = CCMenu::create();
        m_tabButtonMenu->setLayout(RowLayout::create()->setGap(Constants::TabMenu::GAP)->setGrowCrossAxis(true)->setAxisAlignment(AxisAlignment::Start));
        m_tabButtonMenu->setContentSize({ contentWidth, contentHeight * Constants::TabMenu::HEIGHT });
        m_tabButtonMenu->setID("TabButtonMenu"_spr);
        m_tabButtonMenu->setAnchorPoint({ 0.f, 1.f });
        m_tabButtonMenu->ignoreAnchorPointForPosition(false);
        m_tabButtonMenu->setPosition({ 0.f, contentHeight });
        addChild(m_tabButtonMenu);

        m_tabButtonMenuBG = NineSlice::create("square02b_small.png");
        m_tabButtonMenuBG->setOpacity(50);
        m_tabButtonMenuBG->setContentSize({ winSize.width * 1.1f, contentHeight * Constants::TabMenu::HEIGHT });  // to not show rounded corners
        m_tabButtonMenuBG->setAnchorPoint({ 0.5f, 1.f });
        m_tabButtonMenuBG->setPosition({ contentWidth / 2.f, contentHeight });

        addChild(m_tabButtonMenuBG);

        updateLayout();

        return true;
    }

    ChatMenu::~ChatMenu() {
        log::info("ChatMenu instance destroyed");
    }

    void ChatMenu::updateZOrder(int zOrder) {
        setZOrder(zOrder);

        m_tabButtonMenuBG->setZOrder(zOrder + 1);
        m_tabButtonMenu->setZOrder(zOrder + 1);

        auto size { m_tabButtonMenu->getChildrenCount() };
        for (auto i { 0uz }; i < size; ++i) {
            static_cast<TabButton*>(m_tabButtonMenu->getChildByIndex(i))->updateZOrder(zOrder + 2);
        }

        updateLayout();
    }

    void ChatMenu::goToUser(GJUserScore* user) {
        auto tabButton { static_cast<TabButton*>(m_tabButtonMenu->getChildByID(user->m_userName)) };
        if (tabButton) return;
        else {
            auto [contentWidth, contentHeight] { getContentSize() };
            tabButton = TabButton::create((contentWidth / Constants::TabMenu::ROW_LENGTH) - Constants::TabMenu::GAP, contentHeight * Constants::TabMenu::HEIGHT, user);
            m_tabButtonMenu->addChild(tabButton);
            tabButton->updateZOrder(m_tabButtonMenu->getZOrder() + 1);

            m_tabButtonMenu->updateLayout();
        }
    }
}
