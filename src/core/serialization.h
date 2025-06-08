#pragma once

#include <stack>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/reader.h>
#include <rapidjson/document.h>
#include "math/bounding_box.h"

namespace Serialization
{

class BinaryStream
{
public:
    BinaryStream() = default;
    ~BinaryStream() = default;

    BinaryStream(std::vector<uint8_t>& data);

    void write(const uint8_t* data, uint32_t size);

    uint8_t* read(uint32_t size);

    uint8_t* get_data() { return _data.data(); }

    uint32_t get_size() { return _data.size(); }

    void resize(uint32_t size) { _data.resize(size); }

private:
    uint32_t _position = 0;
    std::vector<uint8_t> _data;
};

void w_vec2(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec2 v);
void w_vec3(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec3 v);
void w_vec4(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, glm::vec4 v);
void w_aabb(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, BoundingBox v);

glm::vec2 r_vec2(const rapidjson::Value& value);
glm::vec3 r_vec3(const rapidjson::Value& value);
glm::vec4 r_vec4(const rapidjson::Value& value);
BoundingBox r_aabb(const rapidjson::Value& value);
}