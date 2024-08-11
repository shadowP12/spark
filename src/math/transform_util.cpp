#include "transform_util.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace TransformUtil
{
glm::mat4 remove_scale(const glm::mat4& mat)
{
    glm::vec3 scale;
    glm::quat rot;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, rot, translation, skew, perspective);
    return glm::translate(glm::mat4(1.0), translation) * glm::toMat4(rot);
}
}