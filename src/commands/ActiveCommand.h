#pragma once

#include "MenuItem.h"

class ActiveCommand final : public MenuItem {
public:
    std::string name() const override { return "active"; }
    std::string help() const override {
        return "ACTIVE <0/1 | true/false>\n    changes active state";
    }
    bool execute(Device& device, std::stringstream& args) override;
};

