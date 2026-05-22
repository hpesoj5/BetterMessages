#include "Globals.hpp"
#include "Helpers.hpp"
#include <charconv>
#include <arc/time/Sleep.hpp>

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

    arc::Future<web::WebResponse> sendRequest(web::WebRequest& req, std::string const& endpoint) {
        bool shouldLoop { true };
        while (shouldLoop) {
            asp::Instant until;
            {
                auto nextRequestTime { co_await Globals::Requests::requestMtx.lock() };
                until = *nextRequestTime;
                auto now { asp::Instant::now() };
                if (until <= now) {
                    shouldLoop = false;
                    *nextRequestTime = now + Globals::Requests::REQUEST_DELAY;
                }
            }
            if (shouldLoop) co_await arc::sleepUntil(until);
        }
        // log::info("sending request to {}", endpoint);
        co_return co_await req.post(endpoint);
    }

    std::optional<int> accountIDForUserID(int userID) {
        auto it { Globals::Accounts::accountIDs.find(userID) };
        if (it == Globals::Accounts::accountIDs.end()) return {};
        return it->second;
    }

    void setAccountIDForUserID(int userID, int accountID) { Globals::Accounts::accountIDs[userID] = accountID; }
}
