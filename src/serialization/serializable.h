#pragma once

#include "binary_stream.h"
#include "serialization.h"

class Serializable
{
public:
    Serializable() = default;
    ~Serializable() = default;

    virtual void serialize(SerializationContext& ctx, BinaryStream& bin_stream){};
    virtual void deserialize(DeserializationContext& ctx, BinaryStream& bin_stream){};
};