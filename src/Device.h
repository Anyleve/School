#pragma once

#include <string>

struct Device {
    std::string ip;
    std::string imei;
    std::string imsi;
    std::string config;
    std::string nodes;
    std::string prot = "json";

    double x = 0;
    double y = 0;
    double z = 0;

    int port = 0;

    bool active = false;
    bool should_exit = false;
};

