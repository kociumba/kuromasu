#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "common.h"

#define KUROMASU_SAVE_VERSION 1  // backwards compatible, forwards incompatible

enum class marshal_error {
    OK,
    FORMAT_VERSION_NEWER,
    NO_VERSION,
    WRONG_DATA,
    INVALID_JSON,
    GENERATION_DIFFERS
};

inline const char* get_marshal_error_message(marshal_error err) noexcept {
    switch (err) {
        case marshal_error::OK:
            return "ok";
        case marshal_error::FORMAT_VERSION_NEWER:
            return "Format version is newer than supported";
        case marshal_error::NO_VERSION:
            return "No version information found";
        case marshal_error::WRONG_DATA:
            return "Data format or content is incorrect";
        case marshal_error::INVALID_JSON:
            return "Invalid JSON structure";
        case marshal_error::GENERATION_DIFFERS:
            return "Generated board is different than saved state";

        default:
            return "n/a";
    }
}

struct obs {
    size_t x;
    size_t y;
    int value;
};

using json = nlohmann::json;

void to_json(json& j, const obs& c);
void from_json(const json& j, obs& c);

std::string marshal(ctx_t* ctx);
marshal_error unmarshal(ctx_t* ctx, std::string data, bool force = false);

#endif /* SERIALIZATION_H */
