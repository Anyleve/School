#include "distance_wire.h"

#include "json.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::optional<DistanceWireKind> parse_distance_wire_kind(const std::string& s) {
    const std::string k = lower(s);
    if (k == "json") {
        return DistanceWireKind::Json;
    }
    if (k == "binary") {
        return DistanceWireKind::Binary;
    }
    return std::nullopt;
}

std::string encode_client_position_json(const Vec3& v) {
    nlohmann::json j;
    j["x"] = v.x;
    j["y"] = v.y;
    j["z"] = v.z;
    return j.dump();
}

bool decode_client_position_json(const std::string& line, Vec3& out, std::string& err) {
    err.clear();
    try {
        const nlohmann::json j = nlohmann::json::parse(line);
        if (j.contains("location") and j["location"].is_array() and j["location"].size() >= 3) {
            out.x = j["location"][0].get<double>();
            out.y = j["location"][1].get<double>();
            out.z = j["location"][2].get<double>();
            return true;
        }
        if (j.contains("x") and j.contains("y") and j.contains("z")) {
            out.x = j["x"].get<double>();
            out.y = j["y"].get<double>();
            out.z = j["z"].get<double>();
            return true;
        }
        err = "expected {\"x\",\"y\",\"z\"} or {\"location\":[x,y,z]}";
        return false;
    } catch (const std::exception& e) {
        err = e.what();
        return false;
    }
}

std::string encode_server_distance_json(double d) {
    nlohmann::json j;
    j["distance"] = d;
    return j.dump();
}

std::string encode_server_error_json(const std::string& message) {
    nlohmann::json j;
    j["error"] = message;
    return j.dump();
}

bool decode_server_distance_json(const std::string& line, double& out, std::string& err) {
    err.clear();
    out = 0;
    try {
        const nlohmann::json j = nlohmann::json::parse(line);
        if (j.contains("error")) {
            err = j["error"].get<std::string>();
            return false;
        }
        if (!j.contains("distance")) {
            err = "missing distance";
            return false;
        }
        out = j["distance"].get<double>();
        return true;
    } catch (const std::exception& e) {
        err = e.what();
        return false;
    }
}

void encode_client_position_binary(const Vec3& v, unsigned char out[24]) {
    static_assert(sizeof(double) == 8, "binary wire assumes 64-bit IEEE double");
    std::memcpy(out + 0, &v.x, 8);
    std::memcpy(out + 8, &v.y, 8);
    std::memcpy(out + 16, &v.z, 8);
}

bool decode_client_position_binary(const unsigned char buf[24], Vec3& out) {
    std::memcpy(&out.x, buf + 0, 8);
    std::memcpy(&out.y, buf + 8, 8);
    std::memcpy(&out.z, buf + 16, 8);
    return std::isfinite(out.x) and std::isfinite(out.y) and std::isfinite(out.z);
}

void encode_server_distance_binary(double d, unsigned char out[8]) { std::memcpy(out, &d, 8); }

bool decode_server_distance_binary(const unsigned char buf[8], double& out) {
    std::memcpy(&out, buf, 8);
    return std::isfinite(out);
}
