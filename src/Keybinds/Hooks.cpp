#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <ChatLayer.hpp>

using namespace geode::prelude;

class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double d) {
        if (key == enumKeyCodes::KEY_F9 && isKeyDown && !isKeyRepeat) {
            ChatLayer::get()->toggleOpen();

            return false;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, d);
    }
};
