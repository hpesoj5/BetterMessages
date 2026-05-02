// #include "Constants.hpp"
// #include "ChatMenu.hpp"

// namespace BetterMessages {
//     Ref<ChatMenu> ChatMenu::get() {
//         static Ref<ChatMenu> menu { create() };
//         return menu;
//     }

//     ChatMenu* ChatMenu::create() {
//         auto ptr { new ChatMenu };
//         if (ptr && ptr->init()) {
//             log::info("ChatMenu instance created");
//             return ptr;
//         }

//         CC_SAFE_DELETE(ptr);
//         return nullptr;
//     }

//     bool ChatMenu::init() {
//         if (!CCMenu::init()) return false;

//         setID("chat-menu"_spr);

//         auto winSize { CCDirector::get()->getWinSize() };
//         setContentSize({ winSize.width - 2 * Constants::ChatLayer::PADDING, winSize.height / 2 - Constants::ChatLayer::PADDING });
//         setAnchorPoint({ 0.5f, 0.5f });

//         return true;
//     }
// }
