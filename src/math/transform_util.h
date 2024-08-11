#pragma once

#include "math_define.h"
#include <glm/glm.hpp>

namespace TransformUtil
{
inline glm::vec3 get_right(const glm::mat4& mat)
{
    return glm::vec3(mat[0][0], mat[0][1], mat[0][2]);
}

inline glm::vec3 get_up(const glm::mat4& mat)
{
    return glm::vec3(mat[1][0], mat[1][1], mat[1][2]);
}

inline glm::vec3 get_front(const glm::mat4& mat)
{
    return glm::vec3(mat[2][0], mat[2][1], mat[2][2]);
}

inline glm::vec3 get_translation(const glm::mat4& mat)
{
    return glm::vec3(mat[3][0], mat[3][1], mat[3][2]);
}

glm::mat4 remove_scale(const glm::mat4& mat);
}