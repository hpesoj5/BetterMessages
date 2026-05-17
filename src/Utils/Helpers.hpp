#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace BetterMessages {
    float getScaleFromLength(size_t n);
    int stoi(std::string_view str);
    std::string xor_cycle(std::vector<std::uint8_t> const& input, std::string_view key);
    std::string xor_cycle(std::string_view input, std::string_view key);
    asp::time::Duration timeToNextRequest();
    void setLastRequestTime();
    std::optional<int> accountIDForUserID(int userID);
    void setAccountIDForUserID(int userID, int accountID);
}
