// #include "ChatMenu.hpp"

// namespace Row {
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

//         return true;
//     }
// }
