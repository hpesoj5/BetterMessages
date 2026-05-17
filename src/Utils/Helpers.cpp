#include "Globals.hpp"
#include "Helpers.hpp"
#include <charconv>

namespace BetterMessages {
    float getScaleFromLength(size_t n) {
        if (n < 10) return 0.45f;
        else if (n < 12) return 0.4f;
        else if (n < 16) return 0.35f;
        else return 0.3f;
    }

    int stoi(std::string_view str) {
        int result {};
        std::from_chars(str.data(), str.data() + str.size(), result);
        return result;
    }

    std::string xor_cycle(std::vector<std::uint8_t> const& input, std::string_view key) {
        std::string result {};
        auto size { input.size() };
        auto keySize { key.size() };
        result.reserve(size);
        for(auto i { 0uz }; i < size; ++i) {
            std::uint8_t k = static_cast<uint8_t>(key[i % keySize]);
            result += static_cast<char>(static_cast<std::uint8_t>(input[i]) ^ k);
        }
        return result;
    }

    std::string xor_cycle(std::string_view input, std::string_view key) {
        std::string result {};
        auto size { input.size() };
        auto keySize { key.size() };
        result.reserve(size);
        for(auto i { 0uz }; i < size; ++i) {
            std::uint8_t k = static_cast<uint8_t>(key[i % keySize]);
            result += static_cast<char>(static_cast<std::uint8_t>(input[i]) ^ k);
        }
        return result;
    }

    asp::Duration timeToNextRequest() {
        auto elapsed { Globals::Requests::lastRequestTime.elapsed() };
        if (elapsed >= Globals::Requests::REQUEST_DELAY) return asp::Duration::zero();
        return Globals::Requests::REQUEST_DELAY - elapsed;
    }

    void setLastRequestTime() { Globals::Requests::lastRequestTime = asp::Instant::now(); }

    std::optional<int> accountIDForUserID(int userID) {
        auto it { Globals::Accounts::accountIDs.find(userID) };
        if (it == Globals::Accounts::accountIDs.end()) return {};
        return it->second;
    }

    void setAccountIDForUserID(int userID, int accountID) { Globals::Accounts::accountIDs[userID] = accountID; }
}
