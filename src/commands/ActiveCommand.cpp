#include "commands/ActiveCommand.h"

#include "Device.h"
#include "Logger.h"

#include <algorithm>

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool ActiveCommand::execute(Device& device, std::stringstream& args) {
    std::string value;
    args >> value;
    value = lower(value);

    if (value == "1" || value == "true") {
        device.active = true;
        Logger::instance().info("Command: active true");
        return true;
    }
    if (value == "0" || value == "false") {
        device.active = false;
        Logger::instance().info("Command: active false");
        return true;
    }

    Logger::instance().warn("Command: active invalid arg '" + value + "'");
    return false;
}

