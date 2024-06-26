#include "serialization.h"

namespace Serialization
{

BinaryStream::BinaryStream(std::vector<uint8_t>& data)
{
    _data = std::move(data);
}

void BinaryStream::write(const uint8_t* data, uint32_t size)
{
    _data.insert(_data.end(), data, data + size);
}

uint8_t* BinaryStream::read(uint32_t size)
{
    uint8_t* out_data = _data.data() + _position;
    _position += size;
    return out_data;
}

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
    writer.Double(v[3]);
    writer.EndArray();
}

glm::vec2 r_vec2(const rapidjson::Value& value)
{
    glm::vec2 out = glm::vec2(value[0].GetDouble(), value[1].GetDouble());
    return out;
}

glm::vec3 r_vec3(const rapidjson::Value& value)
{
    glm::vec3 out = glm::vec3(value[0].GetDouble(), value[1].GetDouble(), value[2].GetDouble());
    return out;
}

glm::vec4 r_vec4(const rapidjson::Value& value)
{
    glm::vec4 out = glm::vec4(value[0].GetDouble(), value[1].GetDouble(), value[2].GetDouble(), value[3].GetDouble());
    return out;
}
}