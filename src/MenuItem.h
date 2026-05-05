#pragma once

#include <sstream>
#include <string>

struct Device;

class MenuItem {
public:
    virtual ~MenuItem() = default;

    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::string help() const = 0;


    virtual bool execute(Device& device, std::stringstream& args) = 0;
};

