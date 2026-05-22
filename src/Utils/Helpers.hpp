#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace geode::prelude;

namespace BetterMessages {
    float getScaleFromLength(size_t n);
    int stoi(std::string_view str);
    std::string xor_cycle(std::vector<std::uint8_t> const& input, std::string_view key);
    std::string xor_cycle(std::string_view input, std::string_view key);
    arc::Future<web::WebResponse> sendRequest(web::WebRequest req, std::string endpoint);
    std::optional<int> accountIDForUserID(int userID);
    void setAccountIDForUserID(int userID, int accountID);
}
