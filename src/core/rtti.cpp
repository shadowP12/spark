#include "rtti.h"
#include "hash.h"

RTTI::RTTI(const char* name, int size, RTTI::CreateObjectFunc create_object_func, RTTI::DestructObjectFunc destruct_object_func, CreateRTTIFunc create_rtti_func)
{
    _name = name;
    _size = size;
    _create = create_object_func;
    _destruct = destruct_object_func;
    _hash = 0;
    hash_combine(_hash, _name);
    create_rtti_func(*this);
}

int RTTI::get_base_class_count() const
{
    return (int)_base_classes.size();
}

const RTTI* RTTI::get_base_class(int idx) const
{
    return _base_classes[idx].rtti;
}

void* RTTI::create_object() const
{
    return _create();
}

void RTTI::destruct_object(void* object) const
{
    _destruct(object);
}

void RTTI::add_base_class(const RTTI* rtti, int offset)
{
    BaseClass base;
    base.rtti = rtti;
    base.offset = offset;
    _base_classes.push_back(base);

    for (auto& a : rtti->_attributes)
    {
        _attributes.push_back(a);
    }
}

bool RTTI::is_kind_of(const RTTI* rtti) const
{
    if (this == rtti)
        return true;

    for (auto& b : _base_classes)
    {
        if (b.rtti->is_kind_of(rtti))
            return true;
    }

    return false;
}

const void* RTTI::cast_to(const RTTI* rtti, const void* object) const
{
    if (this == rtti)
        return object;

    for (auto& b : _base_classes)
    {
        const void* casted = (const void*)(((const uint8_t*)object) + b.offset);

        const void* test = b.rtti->cast_to(rtti, casted);
        if (test != nullptr)
            return test;
    }

    return nullptr;
}

void RTTI::add_attribute(const Attribute& attribute)
{
    _attributes.push_back(attribute);
}

const Attribute& RTTI::get_attribute(int idx) const
{
    return _attributes[idx];
}

int RTTI::get_attribute_count() const
{
    return (int)_attributes.size();
}