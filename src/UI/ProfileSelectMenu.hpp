#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class ProfileSelectMenu final : public CCMenu, public UserListDelegate, public TextInputDelegate {
    public:
        static Ref<ProfileSelectMenu> get();

        void updateZOrder();
        void retrieveFriends();
        void updateUsers(CCArray* users);
        void defocusInput();


    private:
        ProfileSelectMenu() = default;
        ~ProfileSelectMenu();

        ProfileSelectMenu(ProfileSelectMenu const&) = delete;
        ProfileSelectMenu(ProfileSelectMenu&&) = delete;
        ProfileSelectMenu& operator=(ProfileSelectMenu const&) = delete;
        ProfileSelectMenu& operator=(ProfileSelectMenu&&) = delete;

        static ProfileSelectMenu* create();
        bool init() override;

        void getUserListFinished(CCArray* scores, UserListType type) override;
        void getUserListFailed(UserListType type, GJErrorCode errorType) override;
        void textChanged(CCTextInputNode* input) override;
        void onClose(CCObject*);

        void onSelectUser(CCObject* sender);
        void displayUsers();

        std::vector<Ref<GJUserScore>> m_users;
        std::vector<Ref<GJUserScore>> m_filteredUsers;

        ScrollLayer* m_profileScrollLayer;
        TextInput* m_searchInput;

        UserListDelegate* m_prevULD;
    };
}
