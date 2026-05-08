#include "ChatLayer.hpp"
#include "ChatHandler.hpp"
#include "Constants.hpp"
#include <Geode/Geode.hpp>

using Constants::ChatHandler::PollRate;

$on_mod(Loaded) {
    auto chatLayer { BetterMessages::ChatLayer::get() };

    arc::Notify notify;
    async::spawn([chatLayer, notify] -> arc::Future<> {
        auto lastLoadTime { asp::Instant::now() };
        while (true) {
            co_await arc::select(
                arc::selectee(
                    arc::sleep(asp::time::Duration::fromSecs(
                        static_cast<int>(chatLayer->isOpen() ? PollRate::Active : PollRate::Background)
                    )),
                    [] { log::info("Timer ended"); }
                ),
                arc::selectee(
                    chatLayer->m_openNotif.notified(),
                    [] { log::info("ChatLayer opened"); }
                )
            );

            auto elapsed { lastLoadTime.elapsed() };

            if ((chatLayer->isOpen() && elapsed >= asp::Duration::fromSecs(static_cast<int>(PollRate::Active)) || elapsed >= asp::Duration::fromSecs(static_cast<int>(PollRate::Background)))) {
                lastLoadTime = asp::Instant::now();
                notify.notifyAll();
            }
        }
    });

    async::spawn([notify] -> arc::Future<> {
        while (true) {
            co_await BetterMessages::ChatHandler::get()->loadMessages();
            co_await notify.notified();
        }
    });
}
