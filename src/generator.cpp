#include "kuromasu.h"

size_t raycast_direction_white(state_t& s, ktl::pos2_size p, size_t dx, size_t dy) {
    size_t count = 0;
    int cx = (int)p.x + dx;
    int cy = (int)p.y + dy;
    while (cx >= 0 && cx < (int)s.game.width && cy >= 0 && cy < (int)s.game.height) {
        cell c = s.game.at((size_t)cx, (size_t)cy);
        switch (c.type) {
            case cell::white:
                count++;
                break;
            default:
                return count;
        }
        cx += dx;
        cy += dy;
    }

    return count;
}

size_t raycast_direction_observers_from_black(state_t& s, ktl::pos2_size start, int dx, int dy) {
    size_t count = 0;

    int cx = (int)start.x + dx;
    int cy = (int)start.y + dy;

    while (s.game.in_bounds((size_t)cx, (size_t)cy)) {
        cell& c = s.game.xy((size_t)cx, (size_t)cy);

        if (c.type == cell::black) { break; }

        if (c.observer_value != -1) { count++; }
        cx += dx;
        cy += dy;
    }

    return count;
}

size_t visible_white(state_t& s, ktl::pos2_size p) {
    if (!s.game.in_bounds(p)) { return -1; }

    size_t visible = 1;  // self

    visible += raycast_direction_white(s, p, -1, 0);
    visible += raycast_direction_white(s, p, 1, 0);
    visible += raycast_direction_white(s, p, 0, -1);
    visible += raycast_direction_white(s, p, 0, 1);

    return visible;
}

uint32_t generate_board(state_t& s,
    std::optional<uint32_t> seed,
    float black_chance,
    float observer_chance) {
    // 1. prepare seed and rng
    std::mt19937 engine;
    uint32_t u_seed;

    if (seed) {
        u_seed = *seed;
    } else {
        std::random_device rd;
        u_seed = rd();
    }

    engine.seed(u_seed);

    std::bernoulli_distribution black_rng(black_chance / 100.0f);

    // 2. reset board
    s.game.fill(cell{.type = cell::white});

    // 3. place random black
    s.game.traverse(
        {0, 0},
        [&](cell&, ktl::pos2_size p) -> bool {
            if (black_rng(engine)) {
                bool black_n = false;
                s.game.orthogonal_neighbors(p, [&](cell& c, ktl::pos2_size) -> bool {
                    if (c.type == cell::black) { black_n = true; }
                    return true;
                });
                if (!black_n) return true;
            }
            return false;
        },
        [&](cell& c, ktl::pos2_size) -> bool {
            cell::type_t old_t = c.type;
            c.type = cell::black;

            // TODO: optimize this later, already probably enough
            if (!s.game.is_connected([&](cell& c, ktl::pos2_size) -> bool {
                    if (c.type == cell::white) { return true; }
                    return false;
                })) {
                c.type = old_t;
            }

            return true;
        });

    std::bernoulli_distribution observer_rng(observer_chance / 100.0f);

    // 4. place random observers
    s.game.traverse(
        {0, 0},
        [&](cell& c, ktl::pos2_size p) -> bool {
            return c.type == cell::white && observer_rng(engine);
        },
        [&](cell& c, ktl::pos2_size p) -> bool {
            c.observer_value = visible_white(s, p);
            return true;
        });

    // 5. reject impossible blacks
    s.game.traverse(
        {0, 0},
        [&](cell& c, ktl::pos2_size p) -> bool { return c.type == cell::black; },
        [&](cell& c, ktl::pos2_size p) -> bool {
            int visible = 0;
            visible += raycast_direction_observers_from_black(s, p, -1, 0);
            visible += raycast_direction_observers_from_black(s, p, 1, 0);
            visible += raycast_direction_observers_from_black(s, p, 0, -1);
            visible += raycast_direction_observers_from_black(s, p, 0, 1);

            if (visible == 0) {
                c.type = cell::white;  // black is unsolvable unset it
            }

            return true;
        });

    s.black_chance = black_chance;
    s.observer_chance = observer_chance;
    
    s.solved_state = s.game;

    // 6. convert solved state into starting position
    s.game.traverse(
        {0, 0},
        [&](cell& c, ktl::pos2_size p) -> bool {
            if (c.type == cell::black) { return true; }
            if (c.type == cell::white && c.observer_value == -1) { return true; }
            return false;
        },
        [&](cell& c, ktl::pos2_size p) -> bool {
            c.type = cell::blank;
            return true;
        });

    s.starting_pos = s.game;

    return u_seed;
}