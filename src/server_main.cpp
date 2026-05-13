#include "Logger.h"
#include "Vec3.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
static constexpr socket_t k_invalid_socket = INVALID_SOCKET;
static int close_socket(socket_t s) { return closesocket(s); }
static std::string socket_err_str() { return "WSA error " + std::to_string(WSAGetLastError()); }
#else
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
static constexpr socket_t k_invalid_socket = -1;
static int close_socket(socket_t s) { return close(s); }
static std::string socket_err_str() { return std::strerror(errno); }
#endif

static std::string trim(std::string s) {
    while (!s.empty() and std::isspace(static_cast<unsigned char>(s.front()))) {
        s.erase(s.begin());
    }
    while (!s.empty() and std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
    return s;
}

static bool parse_vec3_line(const std::string& line, Vec3& out) {
    std::istringstream iss(line);
    double a = 0;
    double b = 0;
    double c = 0;
    if (!(iss >> a >> b >> c)) {
        return false;
    }
    std::string tail;
    if (iss >> tail) {
        return false;
    }
    out.x = a;
    out.y = b;
    out.z = c;
    return true;
}

static bool recv_one_byte(socket_t sock, unsigned char& out) {
#if defined(_WIN32)
    const int n = recv(sock, reinterpret_cast<char*>(&out), 1, 0);
#else
    const ssize_t n = recv(sock, &out, 1, 0);
#endif
    return n == 1;
}

static void send_all(socket_t s, const std::string& msg) {
#if defined(_WIN32)
    (void)send(s, msg.data(), static_cast<int>(msg.size()), 0);
#else
    (void)send(s, msg.data(), msg.size(), 0);
#endif
}

static std::string read_line_socket(socket_t sock) {
    std::string line;
    unsigned char ch = 0;
    while (recv_one_byte(sock, ch)) {
        if (ch == '\n') {
            break;
        }
        if (ch != '\r') {
            line.push_back(static_cast<char>(ch));
        }
    }
    return line;
}

static void print_usage() {
    std::cerr << "school_server <port> <x> <y> <z>\n";
    std::cerr << "Client sends one line: x y z (spaces), or quit\n";
}

static int run_server(int port, const Vec3& server_pos) {
#if defined(_WIN32)
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        Logger::instance().error("WSAStartup failed");
        return 1;
    }
#endif

    socket_t listen_sock = k_invalid_socket;
#if defined(_WIN32)
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (listen_sock == k_invalid_socket) {
        Logger::instance().error(std::string("socket(): ") + socket_err_str());
#if defined(_WIN32)
        WSACleanup();
#endif
        return 1;
    }

    int yes = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) != 0) {
        Logger::instance().warn(std::string("setsockopt(SO_REUSEADDR): ") + socket_err_str());
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(listen_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        Logger::instance().error(std::string("bind(): ") + socket_err_str());
        close_socket(listen_sock);
#if defined(_WIN32)
        WSACleanup();
#endif
        return 1;
    }

    if (listen(listen_sock, 4) != 0) {
        Logger::instance().error(std::string("listen(): ") + socket_err_str());
        close_socket(listen_sock);
#if defined(_WIN32)
        WSACleanup();
#endif
        return 1;
    }

    Logger::instance().info("listen on " + std::to_string(port));
    std::cout << "port " << port << ", waiting for a client (telnet/nc)\n";

    while (true) {
        sockaddr_in client_addr{};
#if defined(_WIN32)
        int client_len = static_cast<int>(sizeof(client_addr));
#else
        socklen_t client_len = sizeof(client_addr);
#endif
        const socket_t client_sock = accept(listen_sock, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client_sock == k_invalid_socket) {
            Logger::instance().error(std::string("accept(): ") + socket_err_str());
            continue;
        }

        char ipbuf[INET_ADDRSTRLEN] = {};
        const char* peer = inet_ntop(AF_INET, &client_addr.sin_addr, ipbuf, INET_ADDRSTRLEN);
        Logger::instance().info(std::string("client ") + (peer ? peer : "?"));

        while (true) {
            const std::string raw = read_line_socket(client_sock);
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
            if (!parse_vec3_line(line, parsed)) {
                Logger::instance().warn("bad line: " + line);
                send_all(client_sock, "err: need x y z\n");
                continue;
            }

            Vec3 tmp(std::move(parsed));
            Vec3 user_vec;
            user_vec = std::move(tmp);

            const double d = distance_between(server_pos, user_vec);
            std::ostringstream reply;
            reply << "distance = " << d << "\n";
            const std::string reply_s = reply.str();

            std::cout << "distance = " << d << "\n";
            Logger::instance().info("d=" + std::to_string(d));

            send_all(client_sock, reply_s);
        }

        close_socket(client_sock);
        Logger::instance().info("client off");
    }

    close_socket(listen_sock);
#if defined(_WIN32)
    WSACleanup();
#endif
    return 0;
}

int main(int argc, char* argv[]) {
    Logger::instance().set_log_file("school_server.log");
    Logger::instance().set_min_level(Logger::Level::Info);

    if (argc != 5) {
        print_usage();
        Logger::instance().error("Invalid command line (expected port and position)");
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

    const Vec3 server_position(x, y, z);
    Logger::instance().info("Server position initialized: (" + std::to_string(x) + ", " + std::to_string(y) + ", " +
                            std::to_string(z) + "), port " + std::to_string(port));
    std::cout << "Server position: (" << x << ", " << y << ", " << z << "), port: " << port << "\n";

    return run_server(port, server_position);
}
