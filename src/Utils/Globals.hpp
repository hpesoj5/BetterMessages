#pragma once

#include <arc/sync/Mutex.hpp>
#include <asp/time/Instant.hpp>
#include <asp/time/Duration.hpp>
#include <unordered_map>

namespace Globals {
    namespace Accounts {
        inline std::unordered_map<int, int> accountIDs;
    }

    namespace ChatLayer {
        inline constexpr float PADDING { 10.f };
    }

    namespace TabMenu {
        inline constexpr float ROW_LENGTH { 8.f };  // number of buttons in each row
        inline constexpr float GAP { 1.f };
        inline constexpr float HEIGHT { 0.125f };  // proportion of ChatMenu height
    }

    namespace Chat {
        inline constexpr float NUM_ROWS { 10.f };  // number of lines of messages in the menu, including the chat input
        inline int refreshInterval { 1000 };  // in milliseconds
    }

    namespace Requests {
        inline const std::string SOCIAL_SECRET { "Wmfd2893gb7" };
        inline arc::Mutex<asp::Instant> requestMtx { asp::Instant::now() };
        inline constexpr asp::Duration REQUEST_DELAY { asp::Duration::fromMillis(1000) };
        inline int activeInterval { 5000 };
        inline int backgroundInterval { 30000 };
        inline bool backgroundPollingEnabled { true };
    }
}
