#pragma once

#include <cmath>
#include <glm/glm.hpp>

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define INV_PI 0.31830988618f
#define INV_TWO_PI 0.15915494309f
#define INF std::numeric_limits<float>::infinity()
#define NEG_INF -INF

enum ClockDirection
{
    CLOCKWISE,
    COUNTERCLOCKWISE
};