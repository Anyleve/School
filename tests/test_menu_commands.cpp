#include "Device.h"
#include "Menu.h"

#include <gtest/gtest.h>

TEST(MenuCommands, ActiveCommandTrue) {
    Device d;
    Menu menu;
    EXPECT_TRUE(menu.dispatch(d, "active true"));
    EXPECT_TRUE(d.active);
}

TEST(MenuCommands, ActiveCommandInvalid) {
    Device d;
    Menu menu;
    EXPECT_FALSE(menu.dispatch(d, "active maybe"));
}

TEST(MenuCommands, MoveUpdatesCoordinates) {
    Device d;
    Menu menu;
    EXPECT_TRUE(menu.dispatch(d, "move 1 2 3"));
    EXPECT_DOUBLE_EQ(d.x, 1);
    EXPECT_DOUBLE_EQ(d.y, 2);
    EXPECT_DOUBLE_EQ(d.z, 3);
}

TEST(MenuCommands, ExitSetsFlag) {
    Device d;
    Menu menu;
    EXPECT_TRUE(menu.dispatch(d, "exit"));
    EXPECT_TRUE(d.should_exit);
}

