#include "kuromasu.h"

raycast_res raycast_direction_non_black(state_t& s, ktl::pos2_size p, size_t dx, size_t dy) {
    raycast_res res;
    int cx = (int)p.x + dx;
    int cy = (int)p.y + dy;

    while (s.game.in_bounds(cx, cy)) {
        cell c = s.game.at((size_t)cx, (size_t)cy);

        switch (c.type) {
            case cell::white:
                res.white_count++;
                break;
            case cell::black:
                res.closed = true;
                return res;
            case cell::blank:
                res.closed = false;
                return res;

            default:
                assert(false);
                return res;
        }

        cx += dx;
        cy += dy;
    }

    res.closed = true;
    return res;
}

bool are_adjacent(const ktl::pos2_size& a, const ktl::pos2_size& b) {
    if (a == b) return false;

    auto dx = (a.x > b.x) ? (a.x - b.x) : (b.x - a.x);
    auto dy = (a.y > b.y) ? (a.y - b.y) : (b.y - a.y);

    return (dx + dy == 1);
}

std::pair<ktl::pos2_size, ktl::pos2_size> find_adjacent_pair(
    const std::vector<ktl::pos2_size>& positions) {
    if (positions.size() < 2) return {ktl::pos2_size::invalid(), ktl::pos2_size::invalid()};

    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
            if (are_adjacent(positions[i], positions[j])) { return {positions[i], positions[j]}; }
        }
    }

    return {ktl::pos2_size::invalid(), ktl::pos2_size::invalid()};
}

void solve(state_t& s) {
    zone_scoped_n("solver check");

    // reset
    s.solved = false;
    for (auto&& [c, pos] : s.game.items()) {
        c.mistake = false;
        c.observer_satisfied = false;
    }

    // 1. check if all observers can see their amount
    s.game.traverse(
        {0, 0},
        [&](cell& c, ktl::pos2_size p) -> bool {
            return c.type == cell::white && c.observer_value != -1;
        },
        [&](cell& c, ktl::pos2_size p) -> bool {
            int visible = 1;
            bool all_closed = true;
            constexpr std::array<direction, 4> dirs = {
                direction{-1, 0}, direction{1, 0}, direction{0, -1}, direction{0, 1}};

            for (auto [dx, dy] : dirs) {
                auto r = raycast_direction_non_black(s, p, dx, dy);
                visible += r.white_count;

                if (!r.closed) { all_closed = false; }
            }

            bool is_mistake = false;

            if (all_closed) {
                if (visible != c.observer_value) {
                    is_mistake = true;
                } else {
                    c.observer_satisfied = true;
                }
            } else {
                if (visible > c.observer_value) { is_mistake = true; }
            }

            c.mistake = is_mistake;
            return !is_mistake;
        });

    // 2. check if any 2 black are next to each other
    std::vector<ktl::pos2_size> pos;
    s.game.traverse(
        {0, 0},
        [&](cell& c, ktl::pos2_size p) -> bool { return c.type == cell::black; },
        [&](cell& c, ktl::pos2_size p) -> bool {
            pos.push_back(p);
            return true;
        });

    auto wrong = find_adjacent_pair(pos);

    if (wrong.first != ktl::pos2_size::invalid() || wrong.second != ktl::pos2_size::invalid()) {
        s.game.at(wrong.first).mistake = true;
        s.game.at(wrong.second).mistake = true;
    }

    // 3. check if all white are connected
    auto is_white = [&](cell& c, ktl::pos2_size) -> bool { return c.type == cell::white; };
    auto is_white_or_blank = [&](cell& c, ktl::pos2_size) -> bool {
        return c.type == cell::white || c.type == cell::blank;
    };

    if (!s.game.is_connected(is_white_or_blank)) {
        ktl::pos2_size start;
        for (auto&& [c, pos] : s.game.items()) {
            if (is_white(c, pos)) { c.mistake = true; }
        }
    }

    // 4. check if all blanks can be white, and no black are replaced with white
    std::vector<ktl::pos2_size> blanks;
    int count = 0;
    int mistake_count = 0;
    for (auto&& [c, pos] : s.game.items()) {
        if (c.mistake) { mistake_count++; }
        if (c.type == cell::blank) { blanks.push_back(pos); }

        if (c.type == cell::white) {
            if (s.solved_state.at(pos).type == cell::black) {
                c.mistake = true;
                mistake_count++;
            }
        }
    }

    for (auto pos : blanks) {
        if (s.solved_state.at(pos).type == cell::white) { count++; }
    }

    if (count == blanks.size() && mistake_count == 0) { s.solved = true; }
}