#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class ProfileSelectMenu final : public CCMenu, public TextInputDelegate {
    public:
        static Ref<ProfileSelectMenu> get();

        void saveUsersToDisk();
        void restoreUsersFromDisk();
        void enableScrollWheel(bool enabled = true);

        void updateZOrder(int zOrder);
        void retrieveFriends();
        void resetInput();


    private:
        ProfileSelectMenu() = default;
        ~ProfileSelectMenu();

        ProfileSelectMenu(ProfileSelectMenu const&) = delete;
        ProfileSelectMenu(ProfileSelectMenu&&) = delete;
        ProfileSelectMenu& operator=(ProfileSelectMenu const&) = delete;
        ProfileSelectMenu& operator=(ProfileSelectMenu&&) = delete;

        static ProfileSelectMenu* create();
        bool init() override;

        void textChanged(CCTextInputNode* input) override;
        void onClose(CCObject*);

        void onSelectUser(CCObject* sender);
        void parseFriendString(std::string const& data);
        void displayUsers();

        std::vector<Ref<GJUserScore>> m_users;
        std::vector<Ref<GJUserScore>> m_filteredUsers;

        ScrollLayer* m_profileScrollLayer;
        TextInput* m_searchInput;

        bool m_isLoading {};
    };
}
