#pragma once

#include <vector>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>

class Serializable
{
    Serializable() = default;
    ~Serializable() = default;

    virtual void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer, std::vector<uint8_t>& bin) {};
    virtual void deserialize(const rapidjson::Value& reader, const std::vector<uint8_t>& bin) {};
};