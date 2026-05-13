#include "net_tcp.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstring>
#include <string>

#if defined(_WIN32)
#include <ws2tcpip.h>
static std::string net_last_error() { return "WSA " + std::to_string(WSAGetLastError()); }
#else
#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
static std::string net_last_error() { return std::strerror(errno); }
#endif

#if defined(_WIN32)
static constexpr socket_t k_invalid = INVALID_SOCKET;
static void close_fd(socket_t s) { closesocket(s); }
#else
static constexpr socket_t k_invalid = -1;
static void close_fd(socket_t s) { close(s); }
#endif

bool net_init() {
#if defined(_WIN32)
    WSADATA wsa{};
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
#else
    return true;
#endif
}

void net_shutdown() {
#if defined(_WIN32)
    WSACleanup();
#endif
}

void net_close(socket_t s) {
    if (s != k_invalid) {
        close_fd(s);
    }
}

static bool set_reuseaddr(socket_t s) {
    int yes = 1;
    return setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == 0;
}

bool tcp_listen_socket(int port, socket_t& listen_out) {
#if defined(_WIN32)
    listen_out = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    listen_out = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (listen_out == k_invalid) {
        return false;
    }
    (void)set_reuseaddr(listen_out);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(listen_out, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        close_fd(listen_out);
        listen_out = k_invalid;
        return false;
    }
    if (listen(listen_out, 8) != 0) {
        close_fd(listen_out);
        listen_out = k_invalid;
        return false;
    }
    return true;
}

bool tcp_accept_one(socket_t listen_sock, socket_t& client_out) {
#if defined(_WIN32)
    int addr_len = 0;
#else
    socklen_t addr_len = 0;
#endif
    sockaddr_in client_addr{};
    addr_len = sizeof(client_addr);
    client_out = accept(listen_sock, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
    return client_out != k_invalid;
}

bool tcp_connect(const std::string& host, int port, socket_t& sock_out) {
    sock_out = k_invalid;
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* res = nullptr;
    const std::string port_str = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0 or res == nullptr) {
        return false;
    }

    socket_t s = k_invalid;
    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s == k_invalid) {
            continue;
        }
#if defined(_WIN32)
        if (connect(s, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) {
#else
        if (connect(s, p->ai_addr, static_cast<socklen_t>(p->ai_addrlen)) == 0) {
#endif
            break;
        }
        close_fd(s);
        s = k_invalid;
    }
    freeaddrinfo(res);
    if (s == k_invalid) {
        return false;
    }
    sock_out = s;
    return true;
}

void net_send_all(socket_t s, const void* data, size_t len) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    size_t sent = 0;
    while (sent < len) {
#if defined(_WIN32)
        const int chunk = static_cast<int>(std::min<size_t>(len - sent, static_cast<size_t>(INT_MAX)));
        const int n = send(s, reinterpret_cast<const char*>(bytes + sent), chunk, 0);
#else
        const ssize_t n = send(s, bytes + sent, len - sent, 0);
#endif
        if (n <= 0) {
            return;
        }
        sent += static_cast<size_t>(n);
    }
}

bool net_recv_exact(socket_t s, void* data, size_t len) {
    auto* bytes = static_cast<unsigned char*>(data);
    size_t got = 0;
    while (got < len) {
#if defined(_WIN32)
        const int n = recv(s, reinterpret_cast<char*>(bytes + got), static_cast<int>(len - got), 0);
#else
        const ssize_t n = recv(s, bytes + got, len - got, 0);
#endif
        if (n <= 0) {
            return false;
        }
        got += static_cast<size_t>(n);
    }
    return true;
}

std::string net_read_line(socket_t s) {
    std::string line;
    unsigned char ch = 0;
    for (;;) {
#if defined(_WIN32)
        const int n = recv(s, reinterpret_cast<char*>(&ch), 1, 0);
#else
        const ssize_t n = recv(s, &ch, 1, 0);
#endif
        if (n != 1) {
            break;
        }
        if (ch == '\n') {
            break;
        }
        if (ch != '\r') {
            line.push_back(static_cast<char>(ch));
        }
    }
    return line;
}
