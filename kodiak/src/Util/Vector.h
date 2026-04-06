#pragma once
#include <cmath>

template<typename T>
class Vec2 {
public:
    T x = 0, y = 0;
public:
    Vec2(T x = 0, T y = 0) {
        this->x = x;
        this->y = y;
    };
public:
    auto sub(const Vec2<T> &v) -> Vec2<T> {
        return Vec2<T>(this->x - v.x, this->y - v.y);
    };

    auto add(const Vec2<T> &v) -> Vec2<T> {
        return Vec2<T>(this->x + v.x, this->y + v.y);
    };

    auto div(const Vec2<T> &v) -> Vec2<T> {
        return Vec2<T>(this->x / v.x, this->y / v.y);
    };

    auto mul(const Vec2<T> &v) -> Vec2<T> {
        return Vec2<T>(this->x * v.x, this->y * v.y);
    };

    auto mul(float v) -> Vec2<T> {
        return Vec2<T>(this->x * v, this->y * v);
    };

    auto dist(const Vec2<T> &v) -> float {
        auto dX = this->x - v.x;
        auto dY = this->y - v.y;

        return sqrt(dX * dX + dY * dY);
    };

    auto ToInt() {
        return Vec2<int>(x, y);
    }

    auto operator+(const Vec2<T> Vec) {
		return Vec2(this->x + Vec.x, this->y + Vec.y);
	}

    auto operator==(const Vec2<T> Vec) {
        return this->x == Vec.x && this->y == Vec.y;
    }

	auto operator-(const Vec2<T> Vec) {
		return Vec2(this->x - Vec.x, this->y - Vec.y);
	}
};

template<typename T>
class Vec3 : public Vec2<T> {
public:
    T z = 0;
public:
    Vec3(T x = 0, T y = 0, T z = 0) : Vec2<T>(x, y) {
        this->z = z;
    };

    auto ToFloat() {
        return Vec3<float>(this->x, this->y, z);
    }

    auto add(T x, T y, T z) -> Vec3<T> {
        return Vec3<T>(this->x + x, this->y + y, this->z + z);
    };

    auto add(const Vec3<T> &vec) -> Vec3<T> {
        return Vec3<T>(this->x + vec.x, this->y + vec.y, this->z + vec.z);
    };

    auto add(T v) -> Vec3<T> {
        return Vec3<T>(this->x + v, this->y + v, this->z + v);
    };

    auto sub(T x, T y, T z) -> Vec3<T> {
        return Vec3<T>(this->x - x, this->y - y, this->z - z);
    };

    auto sub(const Vec3<T> &vec) -> Vec3<T> {
        return Vec3<T>(this->x - vec.x, this->y - vec.y, this->z - vec.z);
    };

    auto sub(T v) -> Vec3<T> {
        return Vec3<T>(this->x - v, this->y - v, this->z - v);
    };

    auto floor() -> Vec3<T> {
        return Vec3<T>(std::floor(this->x), std::floor(this->y), std::floor(this->z));
    };

    auto div(T x, T y, T z) -> Vec3<T> {
        return Vec3<T>(this->x / x, this->y / y, this->z / z);
    };

    auto div(const Vec3<T> &vec) -> Vec3<T> {
        return Vec3<T>(this->x / vec.x, this->y / vec.y, this->z / vec.z);
    };

    auto div(T v) -> Vec3<T> {
        return Vec3<T>(this->x / v, this->y / v, this->z / v);
    };

    auto mul(T x, T y, T z) -> Vec3<T> {
        return Vec3<T>(this->x * x, this->y * y, this->z * z);
    };

    auto mul(const Vec3<T> &vec) -> Vec3<T> {
        return Vec3<T>(this->x * vec.x, this->y * vec.y, this->z * vec.z);
    };

    auto lerp(const Vec3<T> &vec, T t) -> Vec3<T> {
        return Vec3<T>(std::lerp(this->x, vec.x, t), std::lerp(this->y, vec.y, t), std::lerp(this->z, vec.z, t));
    };

    auto normalize() -> Vec3<T> {
        float len = length();
        if (len > 0) {
            return div(len);
        }
        return Vec3<T>(this->x, this->y, this->z);
    }

    float length() {
        return sqrtf(this->x * this->x + this->y * this->y + this->z * this->z);
    }

    float lengthSquared() const {
        return this->x * this->x + this->y * this->y + this->z * this->z;
    }

    float dot(const Vec3& other) const {
        return this->x * other.x + this->y * other.y + this->z * other.z;
    }

    auto round(float precision) -> Vec3<T> {
        return Vec3<T>(
            std::round(this->x / precision) * precision,
            std::round(this->y / precision) * precision,
            std::round(this->z / precision) * precision
        );
    }

    bool equal(const Vec3<T> &vec) const {
        return
            std::floor(this->x) == std::floor(vec.x) &&
            std::floor(this->y) == std::floor(vec.y) &&
            std::floor(this->z) == std::floor(vec.z);
    }

    auto mul(T v) -> Vec3<T> {
        return Vec3<T>(this->x * v, this->y * v, this->z * v);
    };
public:
    auto dist(const Vec3<T> pos) const -> float {
        return sqrt((std::pow(this->x - pos.x, 2)) + (std::pow(this->y - pos.y, 2)) + (std::pow(this->z - pos.z, 2)));
    };
};

template<typename T>
class Vec4 : public Vec3<T> {
public:
    T w = 0;
public:
    explicit Vec4(T x = 0, T y = 0, T z = 0, T w = 0) : Vec3<T>(x, y, z) {
        this->w = w;
    };
};

struct AABB {
    Vec3<float> lower;
    Vec3<float> upper;

    AABB() = default;

    AABB(Vec3<float> l, Vec3<float> h) : lower(l), upper(h) {};

    AABB(const AABB &aabb) {
        lower = Vec3<float>(aabb.lower);
        upper = Vec3<float>(aabb.upper);
    }

    AABB(Vec3<float> lower, float width, float height, float eyeHeight) {
        lower = lower.sub(Vec3<float>(width, eyeHeight * 2, width).div(2));
        this->lower = lower;
        this->upper = Vec3<float>{lower.x + width, lower.y + height, lower.z + width};
    }

    bool isFullBlock() {
        auto diff = lower.sub(upper);
        return fabsf(diff.y) == 1 && fabsf(diff.x) == 1 && fabsf(diff.z) == 1;
    }

    AABB expanded(float amount) {
        return {lower.sub(amount), upper.add(amount)};
    }

    AABB expandedXZ(float amount) {
        return {lower.sub(amount, 0.f, amount), upper.add(amount, 0.f, amount)};
    }

    Vec3<float> centerPoint() {
        Vec3<float> diff = upper.sub(lower);
        return lower.add(diff.mul(0.5f));
    }

    bool intersects(const AABB& aabb) {
        return aabb.upper.x > lower.x && upper.x > aabb.lower.x &&
               aabb.upper.y > lower.y && upper.y > aabb.lower.y &&
               aabb.upper.z > lower.z && upper.z > aabb.lower.z;
    }

    bool intersectsXZ(const AABB& aabb) {
        return aabb.upper.x > lower.x && upper.x > aabb.lower.x &&
               aabb.upper.z > lower.z && upper.z > aabb.lower.z;
    }
};