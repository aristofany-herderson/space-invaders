#pragma once
#include <cstdlib>

namespace Rng
{
    inline float range(float lo, float hi)
    {
        return lo + (hi - lo) * (std::rand() / float(RAND_MAX));
    }

    inline bool coin()
    {
        return std::rand() % 2 == 0;
    }
}
