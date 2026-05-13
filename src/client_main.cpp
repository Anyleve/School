#include "Logger.h"
#include "Vec3.h"
#include "distance_wire.h"
#include "net_tcp.h"

#include <cctype>
#include <iostream>
#include <string>

static std::string trim(std::string s) {
    while (!s.empty() and std::isspace(static_cast<unsigned char>(s.front()))) {
        s.erase(s.begin());
    }
    while (!s.empty() and std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
    return s;
}

static void print_usage() {
    std::cerr << "school_client <host> <port> <x> <y> <z> [json|binary]\n";
}

int main(int argc, char* argv[]) {
    Logger::instance().set_log_file("school_client.log");
    Logger::instance().set_min_level(Logger::Level::Info);

    if (argc != 6 and argc != 7) {
        print_usage();
        Logger::instance().error("Invalid command line");
        return 1;
    }

    const std::string host = argv[1];
    int port = 0;
    double x = 0;
    double y = 0;
    double z = 0;
    try {
        port = std::stoi(argv[2]);
        x = std::stod(argv[3]);
        y = std::stod(argv[4]);
        z = std::stod(argv[5]);
    } catch (const std::exception& e) {
        print_usage();
        Logger::instance().error(std::string("Argument parse error: ") + e.what());
        return 1;
    }

    if (port < 1 or port > 65535) {
        Logger::instance().error("Port must be in range 1..65535");
        print_usage();
        return 1;
    }

    DistanceWireKind kind = DistanceWireKind::Json;
    if (argc == 7) {
        const auto opt = parse_distance_wire_kind(argv[6]);
        if (!opt) {
            Logger::instance().error("Protocol must be json or binary");
            print_usage();
            return 1;
        }
        kind = *opt;
    }

    const Vec3 position(x, y, z);

    if (!net_init()) {
        Logger::instance().error("net_init failed");
        return 1;
    }

    socket_t sock{};
    if (!tcp_connect(host, port, sock)) {
        Logger::instance().error("connect failed to " + host + ":" + std::to_string(port));
        std::cerr << "connect failed\n";
        net_shutdown();
        return 1;
    }

    Logger::instance().info("connected " + host + ":" + std::to_string(port));

    if (kind == DistanceWireKind::Json) {
        std::string line = encode_client_position_json(position);
        line.push_back('\n');
        net_send_all(sock, line.data(), line.size());
        const std::string reply = trim(net_read_line(sock));
        double d = 0;
        std::string err;
        if (!decode_server_distance_json(reply, d, err)) {
            Logger::instance().error("server: " + err);
            std::cerr << err << "\n";
            net_close(sock);
            net_shutdown();
            return 1;
        }
        std::cout << d << "\n";
        Logger::instance().info("distance " + std::to_string(d));
    } else {
        unsigned char buf[24]{};
        encode_client_position_binary(position, buf);
        net_send_all(sock, buf, sizeof buf);
        unsigned char out[8]{};
        if (!net_recv_exact(sock, out, sizeof out)) {
            Logger::instance().error("short read from server");
            std::cerr << "short read from server\n";
            net_close(sock);
            net_shutdown();
            return 1;
        }
        double d = 0;
        if (!decode_server_distance_binary(out, d)) {
            Logger::instance().error("invalid distance from server");
            net_close(sock);
            net_shutdown();
            return 1;
        }
        std::cout << d << "\n";
        Logger::instance().info("distance " + std::to_string(d));
    }

    net_close(sock);
    net_shutdown();
    return 0;
}
