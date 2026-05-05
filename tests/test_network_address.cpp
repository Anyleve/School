#include "NetworkAddress.h"

#include <gtest/gtest.h>

TEST(NetworkAddress, ParsesFullAddress) {
    NetworkAddress a("192.168.0.1:8080");
    EXPECT_EQ(a.ip_string(), "192.168.0.1");
    EXPECT_EQ(a.port(), 8080);
    EXPECT_EQ(a.full_address(), "192.168.0.1:8080");
}

TEST(NetworkAddress, RejectsLastOctetZero) {
    EXPECT_THROW(NetworkAddress("10.0.0.0:1234"), std::invalid_argument);
}

TEST(NetworkAddress, RejectsLastOctet254And255) {
    EXPECT_THROW(NetworkAddress("10.0.0.254:1234"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("10.0.0.255:1234"), std::invalid_argument);
}

TEST(NetworkAddress, RejectsPortOutOfRange) {
    EXPECT_THROW(NetworkAddress("10.0.0.1:0"), std::out_of_range);
    EXPECT_THROW(NetworkAddress("10.0.0.1:70000"), std::out_of_range);
}

TEST(NetworkAddress, IanaPortCategory) {
    NetworkAddress sys("10.0.0.1:22");
    NetworkAddress reg("10.0.0.1:8080");
    NetworkAddress dyn("10.0.0.1:55000");
    EXPECT_EQ(sys.port_category(), NetworkAddress::PortCategory::System);
    EXPECT_EQ(reg.port_category(), NetworkAddress::PortCategory::Registered);
    EXPECT_EQ(dyn.port_category(), NetworkAddress::PortCategory::Dynamic);
}

