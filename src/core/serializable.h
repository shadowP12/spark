#pragma once

#include "serialization.h"

class Serializable
{
public:
    Serializable() = default;
    ~Serializable() = default;

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin) {};
    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin) {};
};