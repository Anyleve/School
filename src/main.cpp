#include <fstream>
#include <iostream>
#include <string>

#include "Device.h"
#include "Logger.h"
#include "Menu.h"
#include "NetworkAddress.h"
#include "json.hpp"

static void apply_json(Device& device, const nlohmann::json& data) {
    if (data.contains("ip")) device.ip = data["ip"];
    if (data.contains("port")) device.port = data["port"];
    if (data.contains("imei")) device.imei = data["imei"];
    if (data.contains("imsi")) device.imsi = data["imsi"];
    if (data.contains("location") and data["location"].is_array() and data["location"].size() >= 3) {
        device.x = data["location"][0];
        device.y = data["location"][1];
        device.z = data["location"][2];
    }
}

int main(int argc, char* argv[]) {
    using json = nlohmann::json;

    Logger::instance().set_log_file("school_app.log");
    Logger::instance().set_min_level(Logger::Level::Info);

    if (argc < 2) {
        Logger::instance().error("Missing json file argument");
        std::cerr << "Usage: app <json_file>\n";
        return 1;
    }

    std::ifstream f(argv[1]);
    if (!f.is_open()) {
        Logger::instance().error(std::string("Could not open json file: ") + argv[1]);
        std::cerr << "Could not open json file: " << argv[1] << "\n";
        return 1;
    }

    json data;
    try {
        data = json::parse(f);
    } catch (json::parse_error& e) {
        Logger::instance().error(std::string("JSON parse error: ") + e.what());
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return 1;
    }

    Device device;
    apply_json(device, data);

    if (!device.ip.empty() and device.port != 0) {
        try {
            NetworkAddress addr(device.ip, std::to_string(device.port));
            Logger::instance().info("Loaded address: " + addr.full_address());
        } catch (const std::exception& e) {
            Logger::instance().warn(std::string("Invalid address from config: ") + e.what());
        }
    }

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-a" or arg == "--ip") and i + 1 < argc) {
            device.ip = argv[++i];
        } else if ((arg == "-p" or arg == "--port") and i + 1 < argc) {
            device.port = std::stoi(argv[++i]);
        } else if ((arg == "-e" or arg == "--imei") and i + 1 < argc) {
            device.imei = argv[++i];
        } else if ((arg == "-i" or arg == "--imsi") and i + 1 < argc) {
            device.imsi = argv[++i];
        } else if ((arg == "-k" or arg == "--config") and i + 1 < argc) {
            device.config = argv[++i];
        } else if ((arg == "-n" or arg == "--nodes") and i + 1 < argc) {
            device.nodes = argv[++i];
        } else if ((arg == "-l" or arg == "--loc") and i + 3 < argc) {
            device.x = std::stod(argv[++i]);
            device.y = std::stod(argv[++i]);
            device.z = std::stod(argv[++i]);
        } else {
            Logger::instance().warn("Unknown argument: " + arg);
            std::cout << "Unknown argument: " << arg << "\n";
        }
    }

    if (!device.ip.empty() and device.port != 0) {
        try {
            NetworkAddress addr(device.ip, std::to_string(device.port));
            Logger::instance().info("Effective address: " + addr.full_address());
        } catch (const std::exception& e) {
            Logger::instance().error(std::string("Invalid effective address: ") + e.what());
        }
    }

    Menu menu;

    std::string line;
    while (!device.should_exit) {
        menu.print(device);
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        menu.dispatch(device, line);
    }

    Logger::instance().info("Application exit");
    return 0;
}

