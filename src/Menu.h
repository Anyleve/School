#pragma once

#include "MenuItem.h"

#include <memory>
#include <vector>

struct Device;

class Menu final {
public:
    Menu();

    void print(const Device& device) const;
    bool dispatch(Device& device, const std::string& line) const;

private:
    std::vector<std::unique_ptr<MenuItem>> items_;
};

