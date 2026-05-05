#pragma once

#include "MenuItem.h"

class ExitCommand final : public MenuItem {
public:
    std::string name() const override { return "exit"; }
    std::string help() const override { return "EXIT\n    terminates application"; }
    bool execute(Device& device, std::stringstream& args) override;
};

