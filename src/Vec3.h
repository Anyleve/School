#pragma once

#include <cmath>

class Vec3 {
public:
    double x = 0;
    double y = 0;
    double z = 0;

    Vec3() = default;
    Vec3(double x_, double y_, double z_);

    Vec3(const Vec3& other);
    Vec3& operator=(const Vec3& other);

    Vec3(Vec3&& other) noexcept;
    Vec3& operator=(Vec3&& other) noexcept;

    ~Vec3() = default;
};

double distance_between(const Vec3& a, const Vec3& b);
