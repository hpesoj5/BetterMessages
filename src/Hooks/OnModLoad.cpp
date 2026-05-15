#include "ChatLayer.hpp"
#include "ChatHandler.hpp"
#include "Constants.hpp"
#include <Geode/Geode.hpp>
#include <arc/prelude.hpp>

using Constants::ChatHandler::PollRate;

$on_mod(Loaded) {
    auto chatLayer { BetterMessages::ChatLayer::get() };
    BetterMessages::ChatHandler::get()->loadMessages();
    BetterMessages::ChatHandler::get()->restoreFromDisk();

    listenForKeybindSettingPresses("toggleChat", [](Keybind const& keybind, bool down, bool repeat, double) {
        if (down && !repeat) {
            BetterMessages::ChatLayer::get()->toggleOpen();
        }
    });

    arc::Notify notify;
    async::spawn([chatLayer, notify] -> arc::Future<> {
        auto lastLoadTime { asp::Instant::now() };
        while (true) {
            co_await arc::select(
                arc::selectee(
                    arc::sleep(asp::time::Duration::fromSecs(
                        static_cast<int>(PollRate::Active)
                    ))
                ),
                arc::selectee(chatLayer->m_openNotif.notified())
            );

            auto elapsed { lastLoadTime.elapsed() };

            if (chatLayer->isOpen() && elapsed >= asp::Duration::fromSecs(static_cast<int>(PollRate::Active))) {
                lastLoadTime = asp::Instant::now();
                notify.notifyAll();
            }
        }
    });

    async::spawn([notify] -> arc::Future<> {
        auto ch { BetterMessages::ChatHandler::get() };
        while (true) {
            co_await async::waitForMainThread([ch] { ch->loadMessages(); });
            // co_await async::waitForMainThread([ch] {
                // ch->refreshChat(ch->getActiveUserID());
            // });
            co_await notify.notified();
        }
    });
}
