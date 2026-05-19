#include "Logger.h"
#include "Vec3.h"
#include "distance_wire.h"
#include "net_tcp.h"

#include <cctype>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

static std::string trim(std::string s) {
    while (!s.empty() and std::isspace(static_cast<unsigned char>(s.front()))) {
        s.erase(s.begin());
    }
    while (!s.empty() and std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
    return s;
}

static std::mutex g_console_mutex;

static void print_usage() {
    std::cerr << "school_server <port> <sx> <sy> <sz> [json|binary]\n";
    std::cerr << "  json: one JSON object per line from client; server replies JSON.\n";
    std::cerr << "  binary: client sends 24 bytes (3 doubles), server sends 8 bytes (distance).\n";
}

static void serve_client_json(socket_t client_sock, const Vec3& server_pos) {
    for (;;) {
        const std::string raw = net_read_line(client_sock);
        if (raw.empty()) {
            break;
        }
        const std::string line = trim(raw);
        if (line.empty()) {
            continue;
        }
        if (line == "quit" or line == "QUIT") {
            Logger::instance().info("quit");
            break;
        }

        Vec3 parsed{};
        std::string jerr;
        if (!decode_client_position_json(line, parsed, jerr)) {
            Logger::instance().warn("json: " + jerr);
            const std::string err_line = encode_server_error_json(jerr) + "\n";
            net_send_all(client_sock, err_line.data(), err_line.size());
            continue;
        }

        Vec3 tmp(std::move(parsed));
        Vec3 user_vec;
        user_vec = std::move(tmp);

        const double d = distance_between(server_pos, user_vec);
        {
            const std::lock_guard<std::mutex> lock(g_console_mutex);
            std::cout << "distance = " << d << "\n";
        }
        Logger::instance().info("d=" + std::to_string(d));
        const std::string reply = encode_server_distance_json(d) + "\n";
        net_send_all(client_sock, reply.data(), reply.size());
    }
}

static void serve_client_binary(socket_t client_sock, const Vec3& server_pos) {
    for (;;) {
        unsigned char buf[24]{};
        if (!net_recv_exact(client_sock, buf, sizeof buf)) {
            break;
        }
        Vec3 client_pos{};
        if (!decode_client_position_binary(buf, client_pos)) {
            Logger::instance().warn("binary: invalid vector");
            break;
        }
        const double d = distance_between(server_pos, client_pos);
        {
            const std::lock_guard<std::mutex> lock(g_console_mutex);
            std::cout << "distance = " << d << "\n";
        }
        Logger::instance().info("d=" + std::to_string(d));
        unsigned char out[8]{};
        encode_server_distance_binary(d, out);
        net_send_all(client_sock, out, sizeof out);
    }
}

static void handle_client(socket_t client_sock, Vec3 server_pos, DistanceWireKind kind) {
    if (kind == DistanceWireKind::Json) {
        serve_client_json(client_sock, server_pos);
    } else {
        serve_client_binary(client_sock, server_pos);
    }
    net_close(client_sock);
    Logger::instance().info("client off");
}

static int run_server(int port, const Vec3& server_pos, DistanceWireKind kind) {
    if (!net_init()) {
        Logger::instance().error("net_init failed");
        return 1;
    }

    socket_t listen_sock{};
    if (!tcp_listen_socket(port, listen_sock)) {
        Logger::instance().error("tcp_listen_socket failed");
        net_shutdown();
        return 1;
    }

    Logger::instance().info("listen on " + std::to_string(port));
    std::cout << "port " << port << ", protocol "
              << (kind == DistanceWireKind::Json ? "json" : "binary")
              << ", waiting for clients (parallel)\n";

    for (;;) {
        socket_t client_sock{};
        if (!tcp_accept_one(listen_sock, client_sock)) {
            Logger::instance().error("accept failed");
            continue;
        }
        Logger::instance().info("client connected");

        std::thread client_thread(handle_client, client_sock, server_pos, kind);
        client_thread.detach();
    }
    return 0;
}

int main(int argc, char* argv[]) {
    Logger::instance().set_log_file("school_server.log");
    Logger::instance().set_min_level(Logger::Level::Info);

    if (argc != 5 and argc != 6) {
        print_usage();
        Logger::instance().error("Invalid command line");
        return 1;
    }

    int port = 0;
    double x = 0;
    double y = 0;
    double z = 0;
    try {
        port = std::stoi(argv[1]);
        x = std::stod(argv[2]);
        y = std::stod(argv[3]);
        z = std::stod(argv[4]);
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
    if (argc == 6) {
        const auto opt = parse_distance_wire_kind(argv[5]);
        if (!opt) {
            Logger::instance().error("Protocol must be json or binary");
            print_usage();
            return 1;
        }
        kind = *opt;
    }

    const Vec3 server_position(x, y, z);
    Logger::instance().info("Server position (" + std::to_string(x) + "," + std::to_string(y) + "," +
                            std::to_string(z) + ") port " + std::to_string(port));
    std::cout << "Server position: (" << x << ", " << y << ", " << z << "), port: " << port << "\n";

    return run_server(port, server_position, kind);
}
