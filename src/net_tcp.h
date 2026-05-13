#pragma once

#include <cstddef>
#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
using socket_t = SOCKET;
#else
using socket_t = int;
#endif

bool net_init();
void net_shutdown();

void net_close(socket_t s);

bool tcp_listen_socket(int port, socket_t& listen_out);
bool tcp_accept_one(socket_t listen_sock, socket_t& client_out);
bool tcp_connect(const std::string& host, int port, socket_t& sock_out);

void net_send_all(socket_t s, const void* data, size_t len);
bool net_recv_exact(socket_t s, void* data, size_t len);
std::string net_read_line(socket_t s);
