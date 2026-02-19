#ifndef KUROMASU_H
#define KUROMASU_H

#include "common.h"

#include <array>
#include <optional>
#include <random>
#include <utility>

struct direction {
    int x, y;
};

struct raycast_res {
    size_t white_count = 0;
    bool closed = false;
};

size_t raycast_direction_white(state_t& s, ktl::pos2_size p, size_t dx, size_t dy);
raycast_res raycast_direction_non_black(state_t& s, ktl::pos2_size p, size_t dx, size_t dy);
size_t visible_white(state_t& s, ktl::pos2_size p);

uint32_t generate_board(state_t& s,
    std::optional<uint32_t> seed = std::nullopt,
    float black_chance = 50.0,
    float observer_chance = 50.0);

void solve(state_t& s);

#endif /* KUROMASU_H */
