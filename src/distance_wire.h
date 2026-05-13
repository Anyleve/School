#pragma once

#include "Vec3.h"

#include <optional>
#include <string>

enum class DistanceWireKind { Json, Binary };

std::optional<DistanceWireKind> parse_distance_wire_kind(const std::string& s);

std::string encode_client_position_json(const Vec3& v);
bool decode_client_position_json(const std::string& line, Vec3& out, std::string& err);

std::string encode_server_distance_json(double d);
std::string encode_server_error_json(const std::string& message);
bool decode_server_distance_json(const std::string& line, double& out, std::string& err);

void encode_client_position_binary(const Vec3& v, unsigned char out[24]);
bool decode_client_position_binary(const unsigned char buf[24], Vec3& out);

void encode_server_distance_binary(double d, unsigned char out[8]);
bool decode_server_distance_binary(const unsigned char buf[8], double& out);
