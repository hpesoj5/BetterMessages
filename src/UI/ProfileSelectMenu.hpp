#pragma once

#include "ProfileButtonMenu.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class ProfileSelectMenu final : public CCMenu, public UserListDelegate {
    public:
        static Ref<ProfileSelectMenu> get();

        void updateZOrder();
        void updateUsers(CCArray* users);

        void getUserListFinished(CCArray* scores, UserListType type) override;
        void getUserListFailed(UserListType type, GJErrorCode errorType) override;

    private:
        ProfileSelectMenu() = default;
        ~ProfileSelectMenu();

        ProfileSelectMenu(ProfileSelectMenu const&) = delete;
        ProfileSelectMenu(ProfileSelectMenu&&) = delete;
        ProfileSelectMenu& operator=(ProfileSelectMenu const&) = delete;
        ProfileSelectMenu& operator=(ProfileSelectMenu&&) = delete;

        static ProfileSelectMenu* create();
        bool init() override;

        ScrollLayer* m_profileScrollLayer;
        TextInput* m_searchInput;

        UserListDelegate* m_prevULD;
    };
}
