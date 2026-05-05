#pragma once

#include <cstdint>
#include <optional>
#include <string>

class NetworkAddress final {
public:
    enum class PortCategory { System, Registered, Dynamic };

    NetworkAddress(int64_t ip_address, int port);

    explicit NetworkAddress(const std::string& full_address);

    NetworkAddress(const std::string& ip_address, const std::string& port);

    [[nodiscard]] uint32_t ip_u32() const { return ip_; }
    [[nodiscard]] int port() const { return port_; }

    [[nodiscard]] std::string ip_string() const;
    [[nodiscard]] std::string full_address() const;
    [[nodiscard]] PortCategory port_category() const;

    static std::optional<uint32_t> try_parse_ipv4(const std::string& ip);
    static std::optional<int> try_parse_port(const std::string& port);

private:
    static void validate(uint32_t ip, int port);

    uint32_t ip_ = 0;
    int port_ = 0;
};

