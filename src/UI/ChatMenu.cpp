#include "Constants.hpp"
#include "ChatHandler.hpp"
#include "ChatMenu.hpp"
#include "TabButton.hpp"
#include <ranges>

namespace BetterMessages {
    Ref<ChatMenu> ChatMenu::get() {
        static Ref<ChatMenu> menu { create() };
        return menu;
    }

    ChatMenu* ChatMenu::create() {
        auto ptr { new ChatMenu };
        if (ptr && ptr->init()) {
            return ptr;
        }

        CC_SAFE_DELETE(ptr);
        return nullptr;
    }

    bool ChatMenu::init() {
        if (!CCMenu::init()) return false;

        setID("ChatMenu"_spr);

        auto winSize { CCDirector::get()->getWinSize() };

        setContentSize({ winSize.width, winSize.height / 2.f });
        setAnchorPoint({ 0.f, 0.f }); setPosition(0.f, 0.f);

        auto [contentWidth, contentHeight] { getContentSize() };
        m_tabButtonMenu = CCMenu::create();
        m_tabButtonMenu->setLayout(RowLayout::create()->setGap(Constants::TabMenu::GAP)->setGrowCrossAxis(true)->setAxisAlignment(AxisAlignment::Start));
        m_tabButtonMenu->setContentSize({ contentWidth, contentHeight * Constants::TabMenu::HEIGHT });
        m_tabButtonMenu->setID("TabButtonMenu"_spr);
        m_tabButtonMenu->setAnchorPoint({ 0.f, 1.f });
        m_tabButtonMenu->ignoreAnchorPointForPosition(false);
        m_tabButtonMenu->setPosition(0.f, contentHeight);
        addChild(m_tabButtonMenu);

        m_tabButtonMenuBG = NineSlice::create("square02b_small.png");
        m_tabButtonMenuBG->setOpacity(50);
        m_tabButtonMenuBG->setContentSize({ winSize.width * 1.1f, contentHeight * Constants::TabMenu::HEIGHT });  // to not show rounded corners
        m_tabButtonMenuBG->setAnchorPoint({ 0.5f, 1.f });
        m_tabButtonMenuBG->setPosition(contentWidth / 2.f, contentHeight);

        addChild(m_tabButtonMenuBG);

        m_chatInput = TextInput::create(contentWidth, "Send message...", "chatFont.fnt");
        // m_chatInput->hideBG();
        m_chatInput->setTextAlign(TextInputAlign::Left);
        m_chatInput->setAnchorPoint({ 0.f, 0.f });
        m_chatInput->ignoreAnchorPointForPosition(false);
        m_chatInput->setPosition(0.f, 0.f);
        m_chatInput->setID("ChatInput"_spr);
        m_chatInput->setMaxCharCount(200);
        m_chatInput->setDelegate(this);
        m_chatInput->getInputNode()->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "sendMessage"),
            [this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
                if (down && !repeat) {
                    if (m_focused) enterPressed(m_chatInput->getInputNode());
                }
            }
        );

        addChild(m_chatInput);

        m_chatHistoryLayer = ScrollLayer::create({ contentWidth, contentHeight - m_tabButtonMenu->getContentHeight() - m_chatInput->getContentHeight() }, true, true);

        m_chatHistoryLayer->m_contentLayer->setLayout(ColumnLayout::create()
            ->setGap(5.f)
            ->setPadding({ Constants::ChatLayer::PADDING, Constants::ChatLayer::PADDING/ 2.f, Constants::ChatLayer::PADDING, Constants::ChatLayer::PADDING / 2.f })
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisLineAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0.f)
        );

        m_chatHistoryLayer->m_contentLayer->ignoreAnchorPointForPosition(false);
        m_chatHistoryLayer->ignoreAnchorPointForPosition(false);
        m_chatHistoryLayer->setAnchorPoint({ 0.f, 0.f });
        m_chatHistoryLayer->setPosition(0.f, m_chatInput->getContentHeight());
        m_chatHistoryLayer->setID("ChatHistory"_spr);

        addChild(m_chatHistoryLayer);

        updateLayout();

        m_chatLoadingSpinner = nullptr;

        return true;
    }

    ChatMenu::~ChatMenu() {
        log::debug("ChatMenu instance destroyed");
    }

    void ChatMenu::setLoading(bool loading) { m_isLoading = loading; }
    bool ChatMenu::isLoading() const { return m_isLoading; }

    void ChatMenu::setLoadingSpinner(bool visible) {
        if (m_chatLoadingSpinner) {
            m_chatLoadingSpinner->removeFromParent();
            m_chatLoadingSpinner = nullptr;
        }
        if (visible) {
            auto contentWidth { getContentWidth() };
            m_chatLoadingSpinner = LoadingSpinner::create(contentWidth * 0.03f);
            m_chatLoadingSpinner->setAnchorPoint({ 1.f, 0.f });
            m_chatLoadingSpinner->setPosition(contentWidth - Constants::ChatLayer::PADDING, Constants::ChatLayer::PADDING);
            addChild(m_chatLoadingSpinner, getZOrder() + 2);
            // log::info("Loading spinner added");
        }
        else {
            // log::info("Loading spinner removed");
        }

        updateLayout();
    }

    void ChatMenu::enableScrollWheel(bool enabled) {
        m_chatHistoryLayer->enableScrollWheel(enabled);
        log::debug("ChatHistoryLayer scrollWheel {}", enabled ? "enabled" : "disabled");
    }

    void ChatMenu::updateZOrder(int zOrder) {
        setZOrder(zOrder);

        m_tabButtonMenuBG->setZOrder(zOrder + 1);
        m_tabButtonMenu->setZOrder(zOrder + 1);

        auto size { m_tabButtonMenu->getChildrenCount() };
        for (auto i { 0uz }; i < size; ++i) {
            static_cast<TabButton*>(m_tabButtonMenu->getChildByIndex(i))->updateZOrder(zOrder + 2);
        }

        m_chatInput->setZOrder(zOrder + 1);
        m_chatHistoryLayer->setZOrder(zOrder + 1);
        if (m_chatLoadingSpinner) m_chatLoadingSpinner->setZOrder(zOrder + 2);

        auto chatHistorySize { m_chatHistoryLayer->m_contentLayer->getChildrenCount() };
        for (auto i { 0uz }; i < chatHistorySize; ++i) {
            m_chatHistoryLayer->m_contentLayer->getChildByIndex(i)->setZOrder(zOrder + 2);
        }
        m_chatHistoryLayer->m_contentLayer->updateLayout();
        updateLayout();
    }

    void ChatMenu::goToUser(GJUserScore* user) {
        auto tabButton { static_cast<TabButton*>(m_tabButtonMenu->getChildByTag(user->m_userID)) };
        if (!tabButton) {
            auto [contentWidth, contentHeight] { getContentSize() };
            tabButton = TabButton::create((contentWidth / Constants::TabMenu::ROW_LENGTH) - Constants::TabMenu::GAP, contentHeight * Constants::TabMenu::HEIGHT, user);
            m_tabButtonMenu->addChild(tabButton);
            tabButton->updateZOrder(m_tabButtonMenu->getZOrder() + 1);

            m_tabButtonMenu->updateLayout();
        }

        ChatHandler::get()->switchChat(user->m_userID);
    }

    void ChatMenu::defocus() {
        m_chatInput->defocus();
    }

    std::string ChatMenu::getInputString() const {
        return m_chatInput->getString();
    }

    void ChatMenu::setInputString(std::string const& draft) {
        m_chatInput->setString(draft);
    }

    void ChatMenu::displayChatHistory(std::vector<Ref<GJUserMessage>> const& history) {
        auto cl { m_chatHistoryLayer->m_contentLayer };
        cl->removeAllChildren();
        auto username { GJAccountManager::get()->m_username };
        auto zOrder { m_chatHistoryLayer->getZOrder() };
        auto contentWidth { m_chatHistoryLayer->getContentWidth() };

        for (auto const& message : history | std::views::reverse) {
            std::string line {};
            if (message->m_outgoing) line += static_cast<std::string>(username) + ": ";
            else line += static_cast<std::string>(message->m_username) + ": ";
            line += static_cast<std::string>(message->m_content);

            auto label { CCLabelBMFont::create(line.c_str(), "chatFont.fnt") };
            label->setAnchorPoint({ 0.f, 0.5f });
            cl->addChild(label, zOrder + 1);
            label->setTag(message->m_userID);
            label->setID(numToString(message->m_messageID));
            label->setScale(0.5f);
            label->setWidth(m_chatHistoryLayer->getContentWidth() - 2 * Constants::ChatLayer::PADDING);
            label->setLineBreakWithoutSpace(true);
        }
        cl->updateLayout();
        cl->setContentHeight(std::max(cl->getContentHeight(), m_chatHistoryLayer->getContentHeight()));
    }

    TabButton* ChatMenu::getTabButtonByTag(int tag) {
        return static_cast<TabButton*>(m_tabButtonMenu->getChildByTag(tag));
    }

    void ChatMenu::textInputOpened(CCTextInputNode* node) { m_focused = true; }

    void ChatMenu::textInputClosed(CCTextInputNode* node) { m_focused = false; }

    void ChatMenu::enterPressed(CCTextInputNode* node) {
        std::string str { node->getString() };
        node->setString("");
        auto ch { ChatHandler::get() };
        auto userID { ch->getActiveUserID() };
        // async::spawn(
        //     ch->sendMessage(userID, str),
        //     [ch, userID] {
        //         if (userID == ch->getActiveUserID()) {
        //             ch->refreshChat(userID);
        //         }
        //     }
        // );
    }
}
