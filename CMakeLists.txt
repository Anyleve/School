cmake_minimum_required(VERSION 4.2)
project(school_task)

set(CMAKE_CXX_STANDARD 20)


add_executable(school_app school_app.cpp)

include(FetchContent)

add_library(school_core
        src/Device.cpp
        src/Logger.cpp
        src/Menu.cpp
        src/NetworkAddress.cpp
        src/Vec3.cpp
        src/distance_wire.cpp
        src/commands/ActiveCommand.cpp
        src/commands/ExitCommand.cpp
        src/commands/MoveCommand.cpp
        src/commands/ProtocolCommand.cpp
)

target_include_directories(school_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(school_app src/main.cpp)
target_link_libraries(school_app PRIVATE school_core)

add_executable(school_server
        src/server_main.cpp
        src/net_tcp.cpp
)
target_link_libraries(school_server PRIVATE school_core)
if (WIN32)
    target_link_libraries(school_server PRIVATE ws2_32)
endif ()

add_executable(school_client
        src/client_main.cpp
        src/net_tcp.cpp
)
target_link_libraries(school_client PRIVATE school_core)
if (WIN32)
    target_link_libraries(school_client PRIVATE ws2_32)
endif ()

option(SCHOOL_TASK_BUILD_TESTS "Build unit tests" ON)
if (SCHOOL_TASK_BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(school_tests
            tests/test_network_address.cpp
            tests/test_menu_commands.cpp
    )
    target_link_libraries(school_tests PRIVATE school_core GTest::gtest_main)
    add_test(NAME school_tests COMMAND school_tests)
endif ()

