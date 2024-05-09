#pragma once

#include <stack>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/reader.h>
#include <rapidjson/document.h>

namespace Serialization
{
void w_vec2(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec2 v);
void w_vec3(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec3 v);
void w_vec4(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec4 v);

glm::vec2 r_vec2(const rapidjson::Value& reader);
glm::vec3 r_vec3(const rapidjson::Value& reader);
glm::vec4 r_vec4(const rapidjson::Value& reader);
}