#pragma once

#include "MenuItem.h"

class ProtocolCommand final : public MenuItem {
public:
    std::string name() const override { return "protocol"; }
    std::string help() const override {
        return "PROTOCOL <json/binary>\n    changes data representation method";
    }
    bool execute(Device& device, std::stringstream& args) override;
};

