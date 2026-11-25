//
// Created by andre on 11/11/2026.
//

#ifndef ECE650_A4_RANDOM_H
#define ECE650_A4_RANDOM_H

#include <random>

namespace rgen::random {

    inline int next_int(int min, int max)
    {
        static std::random_device rd;
        static std::mt19937 gen{rd()};

        std::uniform_int_distribution<int> distribution{min, max};

        return distribution(gen);
    }

} // namespace rgen::random

#endif // ECE650_A4_RANDOM_H
