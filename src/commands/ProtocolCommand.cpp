#include "commands/ProtocolCommand.h"

#include "Device.h"
#include "Logger.h"

#include <algorithm>

static std::string lower2(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool ProtocolCommand::execute(Device& device, std::stringstream& args) {
    std::string variation;
    args >> variation;
    variation = lower2(variation);

    if (variation == "json" or variation == "binary") {
        device.prot = variation;
        Logger::instance().info("Command: protocol " + variation);
        return true;
    }

    Logger::instance().warn("Command: protocol invalid arg '" + variation + "'");
    return false;
}

