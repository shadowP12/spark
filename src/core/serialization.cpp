#include "serialization.h"

namespace Serialization
{
void w_vec2(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec2 v)
{
    writer.StartArray();
    writer.Double(v[0]);
    writer.Double(v[1]);
    writer.EndArray();
}

void w_vec3(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec3 v)
{
    writer.StartArray();
    writer.Double(v[0]);
    writer.Double(v[1]);
    writer.Double(v[2]);
    writer.EndArray();
}

void w_vec4(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec4 v)
{
    writer.StartArray();
    writer.Double(v[0]);
    writer.Double(v[1]);
    writer.Double(v[2]);
    writer.Double(v[4]);
    writer.EndArray();
}

glm::vec2 r_vec2(const rapidjson::Value& reader)
{
    glm::vec2 out = glm::vec2(reader[0].GetDouble(), reader[1].GetDouble());
    return out;
}

glm::vec3 r_vec3(const rapidjson::Value& reader)
{
    glm::vec3 out = glm::vec3(reader[0].GetDouble(), reader[1].GetDouble(), reader[2].GetDouble());
    return out;
}

glm::vec4 r_vec4(const rapidjson::Value& reader)
{
    glm::vec4 out = glm::vec4(reader[0].GetDouble(), reader[1].GetDouble(), reader[2].GetDouble(), reader[3].GetDouble());
    return out;
}
}