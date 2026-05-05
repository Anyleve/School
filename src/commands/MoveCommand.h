#pragma once

#include "MenuItem.h"

class MoveCommand final : public MenuItem {
public:
    std::string name() const override { return "move"; }
    std::string help() const override { return "MOVE [x] [y] [z]\n    changes position vector"; }
    bool execute(Device& device, std::stringstream& args) override;
};

