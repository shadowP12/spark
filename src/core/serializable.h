#pragma once

#include <vector>
#include <rapidjson/rapidjson.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>

class Serializable
{
public:
    Serializable() = default;
    ~Serializable() = default;

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::vector<uint8_t>& bin) {};
    virtual void deserialize(const rapidjson::Value& reader, const std::vector<uint8_t>& bin) {};
};