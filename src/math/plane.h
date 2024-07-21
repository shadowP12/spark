#pragma once

#include "math_define.h"
#include <glm/glm.hpp>

class Plane
{
public:
    Plane(const glm::vec3& p_normal, float p_d = 0.0);
    Plane(const glm::vec3& p_normal, const glm::vec3& p_point);
    Plane(const glm::vec3& p_point1, const glm::vec3& p_point2, const glm::vec3& p_point3, ClockDirection p_dir = CLOCKWISE);

    glm::vec3 get_center() const { return normal * d; }

public:
    glm::vec3 normal;
    float d = 0.0f;
};

Plane::Plane(const glm::vec3& p_normal, float p_d)
{
    normal = p_normal;
    d = p_d;
}

Plane::Plane(const glm::vec3& p_normal, const glm::vec3& p_point)
{
    normal = p_normal;
    normal = glm::normalize(normal);
    d = glm::dot(normal, p_point);
}

Plane::Plane(const glm::vec3& p_point1, const glm::vec3& p_point2, const glm::vec3& p_point3, ClockDirection p_dir)
{
    if (p_dir == CLOCKWISE)
    {
        normal = glm::cross(p_point1 - p_point3, p_point1 - p_point2);
    }
    else
    {
        normal = glm::cross(p_point1 - p_point2, p_point1 - p_point3);
    }

    normal = glm::normalize(normal);
    d = glm::dot(normal, p_point1);
}