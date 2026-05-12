#include "ChatHandler.hpp"
#include "Helpers.hpp"
#include <matjson/std.hpp>

using namespace geode::prelude;

template<>
struct matjson::Serialize<Ref<GJUserMessage>> {
    static Result<Ref<GJUserMessage>> fromJson(matjson::Value const& value) {
        Ref<GJUserMessage> obj { GJUserMessage::create() };

        GEODE_UNWRAP_INTO(obj->m_messageID, value["messageID"].asInt());
        GEODE_UNWRAP_INTO(obj->m_accountID, value["accountID"].asInt());
        GEODE_UNWRAP_INTO(obj->m_userID, value["userID"].asInt());
        GEODE_UNWRAP_INTO(obj->m_title, value["title"].asString());
        GEODE_UNWRAP_INTO(obj->m_content, value["content"].asString());
        GEODE_UNWRAP_INTO(obj->m_username, value["username"].asString());
        GEODE_UNWRAP_INTO(obj->m_uploadDate, value["uploadDate"].asString());
        GEODE_UNWRAP_INTO(obj->m_read, value["read"].asBool());
        GEODE_UNWRAP_INTO(obj->m_outgoing, value["outgoing"].asBool());
        GEODE_UNWRAP_INTO(obj->m_toggled, value["toggled"].asBool());

        return Ok(obj);
    }

    static matjson::Value toJson(Ref<GJUserMessage> value) {
        auto obj { matjson::Value() };

        obj["messageID"] = value->m_messageID;
        obj["accountID"] = value->m_accountID;
        obj["userID"] = value->m_userID;
        obj["title"] = value->m_title;
        obj["content"] = value->m_content;
        obj["username"] = value->m_username;
        obj["uploadDate"] = value->m_uploadDate;
        obj["read"] = value->m_read;
        obj["outgoing"] = value->m_outgoing;
        obj["toggled"] = value->m_toggled;

        return obj;
    }
};

template<>
struct matjson::Serialize<BetterMessages::Chat> {
    static Result<BetterMessages::Chat> fromJson(matjson::Value const& value) {
        GEODE_UNWRAP_INTO(std::vector historyJSON, value["history"].asArray());
        GEODE_UNWRAP_INTO(std::string draftMessage, value["draftMessage"].asString());
        GEODE_UNWRAP_INTO(int unreadCount, value["unreadCount"].asInt());

        std::vector<Ref<GJUserMessage>> history;
        for (auto& message : historyJSON) {
            GEODE_UNWRAP_INTO(Ref<GJUserMessage> val, message.as<Ref<GJUserMessage>>());
            history.push_back(val);
        }

        return Ok(BetterMessages::Chat{ history, draftMessage, unreadCount });
    }

    static matjson::Value toJson(BetterMessages::Chat const& value) {
        auto obj { matjson::Value() };

        auto history { matjson::Value::array() };
        for (auto message : value.history) history.push(message);

        obj["history"] = history;
        obj["draftMessage"] = value.draftMessage;
        obj["unreadCount"] = value.unreadCount;
        return obj;
    }
};

// BetterInfo
// https://github.com/geode-sdk/json/blob/d54b2aba2361f08b08d96d1f302b19cf2153c305/include/matjson/std.hpp#L169
template <class T, class Hash, class KeyEqual, class Alloc>
struct matjson::Serialize<std::unordered_map<int, T, Hash, KeyEqual, Alloc>> {
    using Map = std::unordered_map<int, T, Hash, KeyEqual, Alloc>;

    static geode::Result<Map> fromJson(Value const& value)
        requires requires(Value const& value) { value.template as<std::decay_t<T>>(); }
    {
        if (!value.isObject()) return geode::Err("not an object");
        Map res;
        for (auto const& [k, v] : value) {
            GEODE_UNWRAP_INTO(auto vv, v.template as<std::decay_t<T>>());
            res.insert({BetterMessages::stoi(k), vv});
        }
        return geode::Ok(res);
    }

    static Value toJson(Map const& value)
        requires requires(T const& value) { Value(value); }
    {
        Value res;
        for (auto const& [k, v] : value) {
            res.set(fmt::format("{}", k), Value(v));
        }
        return res;
    }
};

