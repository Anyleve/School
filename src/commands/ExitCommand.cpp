#include "commands/ExitCommand.h"

#include "Device.h"
#include "Logger.h"

bool ExitCommand::execute(Device& device, std::stringstream&) {
    Logger::instance().info("Command: exit");
    device.should_exit = true;
    return true;
}

