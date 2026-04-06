#pragma once

struct StateVectorComponent {
    Vec3<float> position{};
    Vec3<float> previous{};
    Vec3<float> motion{};
};
