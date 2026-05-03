#include "Helpers.hpp"

namespace BetterMessages {
    float getScaleFromLength(size_t n) {
        if (n < 10) return 0.45f;
        else if (n < 12) return 0.4f;
        else if (n < 16) return 0.35f;
        else return 0.3f;
    }
}
