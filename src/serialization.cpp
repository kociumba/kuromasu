#include "serialization.h"
#include "kuromasu.h"

void to_json(json& j, const obs& c) { j = json{{"x", c.x}, {"y", c.y}, {"v", c.value}}; }

void from_json(const json& j, obs& c) {
    j.at("x").get_to(c.x);
    j.at("y").get_to(c.y);
    j.at("v").get_to(c.value);
}

std::string marshal(ctx_t* ctx) {
    zone_scoped_n("marshaling board data");

    json doc;

    doc["version"] = KUROMASU_SAVE_VERSION;
    doc["seed"] = ctx->state.seed;
    doc["width"] = ctx->state.starting_pos.width;
    doc["height"] = ctx->state.starting_pos.height;
    doc["black_chance"] = ctx->state.black_chance;
    doc["observer_chance"] = ctx->state.observer_chance;

    std::vector<obs> observers;

    for (auto&& [cell, pos] : ctx->state.starting_pos.items()) {
        if (cell.observer_value != -1) { observers.push_back({pos.x, pos.y, cell.observer_value}); }
    }

    doc["observers"] = observers;

    return doc.dump();
}

marshal_error unmarshal(ctx_t* ctx, std::string data, bool force) {
    zone_scoped_n("unmarshalling data");

    json doc;

    try {
        doc = json::parse(data);  // library forces exception here otherwise crashes on invalid data
    } catch (const json::parse_error&) { return marshal_error::INVALID_JSON; }

    if (!force) {
        if (!doc.contains("version") || !doc["version"].is_number_unsigned()) {
            return marshal_error::NO_VERSION;
        }

        uint32_t file_version = doc["version"].get<uint32_t>();
        if (file_version > KUROMASU_SAVE_VERSION) { return marshal_error::FORMAT_VERSION_NEWER; }
    }

    if (!doc.contains("seed") || !doc["seed"].is_number_unsigned()) {
        return marshal_error::WRONG_DATA;
    }
    ctx->state.seed = doc["seed"].get<uint32_t>();

    if (!doc.contains("width") || !doc["width"].is_number_unsigned() || !doc.contains("height") ||
        !doc["height"].is_number_unsigned()) {
        return marshal_error::WRONG_DATA;
    }

    size_t loaded_w = doc["width"].get<size_t>();
    size_t loaded_h = doc["height"].get<size_t>();

    ctx->state.game.resize(loaded_w, loaded_h);
    ctx->state.game.fill(cell{.type = cell::blank, .observer_value = -1});

    if (doc.contains("black_chance")) {
        if (!doc["black_chance"].is_number()) { return marshal_error::WRONG_DATA; }
        float bc = doc["black_chance"].get<float>();
        if (bc < 0.0f || bc > 100.0f) { return marshal_error::WRONG_DATA; }
        ctx->state.black_chance = bc;
    }

    if (doc.contains("observer_chance")) {
        if (!doc["observer_chance"].is_number()) { return marshal_error::WRONG_DATA; }
        float oc = doc["observer_chance"].get<float>();
        if (oc < 0.0f || oc > 100.0f) { return marshal_error::WRONG_DATA; }
        ctx->state.observer_chance = oc;
    }

    generate_board(
        ctx->state, ctx->state.seed, ctx->state.black_chance, ctx->state.observer_chance);

    if (doc.contains("observers") && doc["observers"].is_array()) {
        const auto& arr = doc["observers"];
        int differences = 0;

        for (const auto& item : arr) {
            if (!item.is_object()) continue;

            size_t x = item.value("x", size_t{0});
            size_t y = item.value("y", size_t{0});
            int val = item.value("v", -1);

            auto pos = ktl::pos2_size{x, y};

            if (!ctx->state.game.in_bounds(pos)) differences++;
            if (!ctx->state.game.xy(x, y).observer_value == val) differences++;
        }

        if (differences != 0) return marshal_error::GENERATION_DIFFERS;
    }

    return marshal_error::OK;
}