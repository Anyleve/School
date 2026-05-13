#include "commands/MoveCommand.h"

#include "Device.h"
#include "Logger.h"

bool MoveCommand::execute(Device& device, std::stringstream& args) {
    double val = 0;
    int count = 0;
    double coords[3] = {device.x, device.y, device.z};

    while (count < 3 and (args >> val)) {
        coords[count] = val;
        count++;
    }

    if (count <= 0) {
        Logger::instance().warn("Command: move missing args");
        return false;
    }

    device.x = coords[0];
    device.y = coords[1];
    device.z = coords[2];

    Logger::instance().info("Command: move updated " + std::to_string(count) + " coordinate(s)");
    return true;
}

