#include "ChatLayer.hpp"
#include "ChatHandler.hpp"
#include "Globals.hpp"
#include <Geode/Geode.hpp>
#include <arc/prelude.hpp>

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
        using Globals::Requests::activeInterval;
        using Globals::Requests::backgroundInterval;
        using Globals::Requests::backgroundPollingEnabled;
        auto lastLoadTime { asp::Instant::now() };
        while (true) {
            co_await arc::select(
                arc::selectee(arc::sleep(asp::Duration::fromMillis(chatLayer->isOpen() ? activeInterval : backgroundInterval))),
                arc::selectee(chatLayer->m_openNotif.notified())
            );

            auto elapsed { lastLoadTime.elapsed() };

            if (chatLayer->isOpen() && elapsed >= asp::Duration::fromMillis(activeInterval) || (backgroundPollingEnabled && elapsed >= asp::Duration::fromMillis(backgroundInterval))) {
                lastLoadTime = asp::Instant::now();
                notify.notifyAll();
            }
        }
    });

    async::spawn([notify] -> arc::Future<> {
        while (true) {
            co_await async::waitForMainThread([] { BetterMessages::ChatHandler::get()->loadMessages(); });
            co_await notify.notified();
        }
    });

    async::spawn([] -> arc::Future<> {
        while (true) {
            co_await async::waitForMainThread([] {
                if (BetterMessages::ChatLayer::get()->isOpen()) {
                    auto ch { BetterMessages::ChatHandler::get() };
                    ch->refreshChat(ch->getActiveUserID());
                }
            });
            co_await arc::sleep(asp::Duration::fromMillis(Globals::Chat::refreshInterval));
        }
    });
}

$on_game(Exiting) {
    BetterMessages::ChatHandler::get()->saveToDisk();
}
