#include "ChatLayer.hpp"
#include "ChatHandler.hpp"
#include "Globals.hpp"
#include <Geode/Geode.hpp>
#include <arc/prelude.hpp>

$on_mod(Loaded) {
    BetterMessages::ChatHandler::get()->loadMessages();
    BetterMessages::ChatHandler::get()->restoreFromDisk();

    listenForKeybindSettingPresses("toggleChat", [](Keybind const& keybind, bool down, bool repeat, double) {
        if (down && !repeat) {
            BetterMessages::ChatLayer::get()->toggleOpen();
        }
    });

    arc::Notify notify;
    async::spawn([notify] -> arc::Future<> {
        using Globals::Requests::activeInterval;
        using Globals::Requests::backgroundInterval;
        using Globals::Requests::backgroundPollingEnabled;
        auto lastLoadTime { asp::Instant::now() };
        arc::Notify* openNotif {};
        co_await async::waitForMainThread([&openNotif] { openNotif = &(BetterMessages::ChatLayer::get()->m_openNotif); });
        while (true) {
            bool isOpen { (co_await async::waitForMainThread<bool>([] { return BetterMessages::ChatLayer::get()->isOpen(); })) };
            co_await arc::select(
                arc::selectee(arc::sleep(asp::Duration::fromMillis(isOpen ? activeInterval : backgroundInterval))),
                arc::selectee(openNotif->notified())
            );

            auto elapsed { lastLoadTime.elapsed() };

            isOpen = (co_await async::waitForMainThread<bool>([] { return BetterMessages::ChatLayer::get()->isOpen(); })).value_or(false);
            if (isOpen && elapsed >= asp::Duration::fromMillis(activeInterval) || (backgroundPollingEnabled && elapsed >= asp::Duration::fromMillis(backgroundInterval))) {
                lastLoadTime = asp::Instant::now();
                notify.notifyAll();
            }
        }
    }).setName("PollCycle-1"_spr);

    async::spawn([notify] -> arc::Future<> {
        while (true) {
            co_await async::waitForMainThread([] { BetterMessages::ChatHandler::get()->loadMessages(); });
            co_await async::waitForMainThread([] {
                if (BetterMessages::ChatLayer::get()->isOpen()) {
                    auto ch { BetterMessages::ChatHandler::get() };
                    ch->refreshChat(ch->getActiveUserID());
                }
            });
            co_await notify.notified();
        }
    }).setName("PollCycle-2"_spr);

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
    }).setName("RefreshChatCycle"_spr);
}

$on_game(Exiting) {
    BetterMessages::ChatHandler::get()->saveToDisk();
}
