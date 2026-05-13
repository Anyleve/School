#include "Vec3.h"

#include "Logger.h"

Vec3::Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

Vec3::Vec3(const Vec3& other) = default;

Vec3& Vec3::operator=(const Vec3& other) = default;

Vec3::Vec3(Vec3&& other) noexcept : x(other.x), y(other.y), z(other.z) {
    Logger::instance().info("vec3: move ctor");
    other.x = 0;
    other.y = 0;
    other.z = 0;
}

Vec3& Vec3::operator=(Vec3&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    Logger::instance().info("vec3: move =");
    x = other.x;
    y = other.y;
    z = other.z;
    other.x = 0;
    other.y = 0;
    other.z = 0;
    return *this;
}

double distance_between(const Vec3& a, const Vec3& b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    const double dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}
