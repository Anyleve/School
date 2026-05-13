#include "NetworkAddress.h"

#include <array>
#include <charconv>
#include <sstream>
#include <stdexcept>

static int to_int_checked(const std::string& s) {
    int value = 0;
    const char* begin = s.data();
    const char* end = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(begin, end, value);
    if (ec != std::errc() or ptr != end) {
        throw std::invalid_argument("not an integer: '" + s + "'");
    }
    return value;
}

NetworkAddress::NetworkAddress(int64_t ip_address, int port) {
    if (ip_address < 0 or ip_address > 0xFFFFFFFFLL) {
        throw std::out_of_range("ip_address out of range (expected 0..2^32-1)");
    }
    ip_ = static_cast<uint32_t>(ip_address);
    port_ = port;
    validate(ip_, port_);
}

NetworkAddress::NetworkAddress(const std::string& full_address) {
    const auto pos = full_address.rfind(':');
    if (pos == std::string::npos) {
        throw std::invalid_argument("full_address must contain ':'");
    }
    const std::string ip_part = full_address.substr(0, pos);
    const std::string port_part = full_address.substr(pos + 1);

    auto ipOpt = try_parse_ipv4(ip_part);
    if (!ipOpt) {
        throw std::invalid_argument("invalid IPv4 address: '" + ip_part + "'");
    }
    auto portOpt = try_parse_port(port_part);
    if (!portOpt) {
        throw std::invalid_argument("invalid port: '" + port_part + "'");
    }

    ip_ = *ipOpt;
    port_ = *portOpt;
    validate(ip_, port_);
}

NetworkAddress::NetworkAddress(const std::string& ip_address, const std::string& port) {
    auto ipOpt = try_parse_ipv4(ip_address);
    if (!ipOpt) {
        throw std::invalid_argument("invalid IPv4 address: '" + ip_address + "'");
    }
    auto portOpt = try_parse_port(port);
    if (!portOpt) {
        throw std::invalid_argument("invalid port: '" + port + "'");
    }

    ip_ = *ipOpt;
    port_ = *portOpt;
    validate(ip_, port_);
}

std::optional<uint32_t> NetworkAddress::try_parse_ipv4(const std::string& ip) {
    std::array<int, 4> octets{};
    int count = 0;

    std::istringstream iss(ip);
    std::string token;
    while (std::getline(iss, token, '.')) {
        if (count >= 4 or token.empty()) {
            return std::nullopt;
        }
        int v = 0;
        try {
            v = to_int_checked(token);
        } catch (...) {
            return std::nullopt;
        }
        if (v < 0 or v > 255) {
            return std::nullopt;
        }
        octets[count++] = v;
    }
    if (count != 4) {
        return std::nullopt;
    }

    const uint32_t value = (static_cast<uint32_t>(octets[0]) << 24) |
                           (static_cast<uint32_t>(octets[1]) << 16) |
                           (static_cast<uint32_t>(octets[2]) << 8) |
                           (static_cast<uint32_t>(octets[3]));
    return value;
}

std::optional<int> NetworkAddress::try_parse_port(const std::string& port) {
    if (port.empty()) {
        return std::nullopt;
    }
    int v = 0;
    try {
        v = to_int_checked(port);
    } catch (...) {
        return std::nullopt;
    }
    return v;
}

void NetworkAddress::validate(uint32_t ip, int port) {
    const uint32_t last_octet = ip & 0xFFu;
    if (last_octet < 1 or last_octet >= 254) {
        throw std::invalid_argument("IPv4 last octet invariant violated (expected 1..253)");
    }
    
    if (port < 1 or port > 65535) {
        throw std::out_of_range("port out of range (expected 1..65535)");
    }
}

std::string NetworkAddress::ip_string() const {
    const uint32_t a = (ip_ >> 24) & 0xFFu;
    const uint32_t b = (ip_ >> 16) & 0xFFu;
    const uint32_t c = (ip_ >> 8) & 0xFFu;
    const uint32_t d = (ip_) & 0xFFu;

    std::ostringstream oss;
    oss << a << "." << b << "." << c << "." << d;
    return oss.str();
}

std::string NetworkAddress::full_address() const {
    std::ostringstream oss;
    oss << ip_string() << ":" << port_;
    return oss.str();
}

NetworkAddress::PortCategory NetworkAddress::port_category() const {
    if (port_ <= 1023) {
        return PortCategory::System;
    }
    if (port_ <= 49151) {
        return PortCategory::Registered;
    }
    return PortCategory::Dynamic;
}

