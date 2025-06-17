#pragma once

#include "math_define.h"
#include <glm/glm.hpp>

namespace TransformUtil {
inline glm::vec3 transform_point(const glm::vec3& point, const glm::mat4& mat)
{
    glm::vec4 p0 = glm::vec4(point.x, point.y, point.z, 1.0f);
    glm::vec4 p1 = mat * p0;
    return glm::vec3(p1.x, p1.y, p1.z) / p1.w;
}

inline glm::vec3 get_x_axis(const glm::mat4& mat)
{
    return glm::vec3(mat[0][0], mat[0][1], mat[0][2]);
}

inline glm::vec3 get_y_axis(const glm::mat4& mat)
{
    return glm::vec3(mat[1][0], mat[1][1], mat[1][2]);
}

inline glm::vec3 get_z_axis(const glm::mat4& mat)
{
    return glm::vec3(mat[2][0], mat[2][1], mat[2][2]);
}

inline glm::vec3 get_translation(const glm::mat4& mat)
{
    return glm::vec3(mat[3][0], mat[3][1], mat[3][2]);
}

glm::mat4 remove_scale(const glm::mat4& mat);
}// namespace TransformUtil