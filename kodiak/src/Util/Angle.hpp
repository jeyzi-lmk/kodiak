#pragma once
#include <cmath>
#include <numbers>
#include <algorithm>

namespace Angle {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 2.0f * PI;

    __forceinline static float normalizeYaw(float yaw) {
        yaw = std::fmod(yaw, TWO_PI);
        if (yaw > PI) yaw -= TWO_PI;
        else if (yaw <= -PI) yaw += TWO_PI;
        return yaw;
    }

    __forceinline static float normalizePitch(float pitch) {
        return std::clamp(pitch, -PI/2.0f + 0.001f, PI/2.0f - 0.001f);
    }

    __forceinline float rad2deg(float radians) {
        return radians * (180.0f / PI);
    }

    __forceinline float deg2rad(float degrees) {
        return degrees * (PI / 180.0f);
    }

    __forceinline float wrapPi(float a) {
        a = std::fmod(a, TWO_PI);
        if (a > PI) a -= TWO_PI;
        else if (a <= -PI) a += TWO_PI;
        return a;
    }

    __forceinline float lerpAngleRad(float current, float target, float t) {
        t = std::clamp(t, 0.f, 1.f);
        float diff = wrapPi(target - current);
        return wrapPi(current + diff * t);
    }

    __forceinline Vec2<float> computeAim(Vec3<float> from, Vec3<float> to, float yOffset = 0.1f) {
        Vec3<float> delta = from.sub(to);
        delta.y += yOffset;

        float distToTarget = delta.length();
        if(distToTarget < 0.001f) return {0.0f, 0.0f};

        float yaw = atan2f(delta.x, delta.z);
        float pitch = -atan2f(delta.y, sqrtf(delta.x * delta.x + delta.z * delta.z));

        yaw = normalizeYaw(yaw);
        pitch = normalizePitch(pitch);

        return { yaw, pitch };
    };
}