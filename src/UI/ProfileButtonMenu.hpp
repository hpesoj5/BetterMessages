#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class ProfileButtonMenu final : public CCMenu {
    public:
        static ProfileButtonMenu* create();
        static ProfileButtonMenu* create(CCArray* users);

        void updateUsers(CCArray* users);
        void updateZOrder(int zOrder);

    private:
        bool init() override;
        bool init(CCArray* users);
    };
}
