#include "Menu.h"

#include "Device.h"
#include "Logger.h"
#include "commands/ActiveCommand.h"
#include "commands/ExitCommand.h"
#include "commands/MoveCommand.h"
#include "commands/ProtocolCommand.h"

#include <algorithm>
#include <iostream>
#include <sstream>

static std::string lower3(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

Menu::Menu() {
    items_.push_back(std::make_unique<ExitCommand>());
    items_.push_back(std::make_unique<ActiveCommand>());
    items_.push_back(std::make_unique<MoveCommand>());
    items_.push_back(std::make_unique<ProtocolCommand>());
}

void Menu::print(const Device& device) const {
    std::cout << "\n====================================\n";
    std::cout << "IMSI: " << device.imsi << "\n";
    std::cout << "ACTIVE: " << (device.active ? "TRUE" : "FALSE") << "\n";
    std::cout << "CURRENT LOCATION: (" << device.x << ", " << device.y << ", " << device.z << ")\n";
    std::cout << "CURRENT PROTOCOL: " << device.prot << "\n";
    std::cout << "\nCOMMANDS:\n";
    for (const auto& item : items_) {
        std::cout << item->help() << "\n\n";
    }
    std::cout << "====================================\n";
}

bool Menu::dispatch(Device& device, const std::string& line) const {
    std::stringstream ss(line);
    std::string cmd;
    ss >> cmd;
    cmd = lower3(cmd);

    if (cmd.empty()) {
        return true;
    }

    for (const auto& item : items_) {
        if (item->name() == cmd) {
            const bool ok = item->execute(device, ss);
            if (!ok) {
                std::cout << "INCORRECT INPUT\n";
            }
            return ok;
        }
    }

    Logger::instance().warn("Unknown command: " + cmd);
    std::cout << "INCORRECT INPUT\n";
    return false;
}

