#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace BetterMessages {
    class TabButton final: public CCMenu {
    public:
        static TabButton* create(float width, float height, GJUserScore* user);

        void updateZOrder(int ZOrder);
        void select(CCObject* = nullptr);
        void setSelectedSprite(bool selected);

    private:
        TabButton() = default;
        ~TabButton() = default;

        bool init(float width, float height, GJUserScore* user);

        void onClose(CCObject*);

        CCMenuItemSpriteExtra* m_button;
        NineSlice* m_selectedSprite;
        CCMenu* m_closeMenu;
        CCMenuItemSpriteExtra* m_close;
        CCLabelBMFont* m_name;
    };
}
