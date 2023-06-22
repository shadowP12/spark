#pragma once

#include "attribute.h"
#include <vector>

class RTTI
{
public:
    using CreateObjectFunc = void *(*)();

    using DestructObjectFunc = void (*)(void *object);

    using CreateRTTIFunc = void (*)(RTTI& rtti);

    RTTI(const char* name, int size, CreateObjectFunc create_object_func, DestructObjectFunc destruct_object_func, CreateRTTIFunc create_rtti_func);

    bool operator == (const RTTI& rhs) const { return this == &rhs; }

    bool operator != (const RTTI& rhs) const { return this != &rhs; }

    inline const char* get_name() const { return _name; }

    inline int get_size() const { return _size; }

    inline std::size_t get_hash() const { return _hash; };

    int get_base_class_count() const;

    const RTTI* get_base_class(int idx) const;

    void* create_object() const;

    void destruct_object(void* object) const;

    void add_base_class(const RTTI* rtti, int offset);

    bool is_kind_of(const RTTI* rtti) const;

    const void* cast_to(const RTTI* rtti, const void* object) const;

    void add_attribute(const Attribute& attribute);

    const Attribute& get_attribute(int idx) const;

    int get_attribute_count() const;

protected:
    struct BaseClass
    {
        const RTTI* rtti;
        int offset;
    };

    const char* _name;
    int _size;
    std::size_t _hash;
    CreateObjectFunc _create;
    DestructObjectFunc _destruct;
    std::vector<BaseClass> _base_classes;
    std::vector<Attribute> _attributes;
};

#define FIND_RTTI(class_name) get_rtti_of_type((class_name*)nullptr)

#define DECLARE_RTTI(linkage, class_name, modifier)                                                                 \
public:                                                                                                             \
    friend linkage RTTI* get_rtti_of_type(class_name*);                                                             \
    friend inline const RTTI* get_rtti(const class_name* object) { return object->get_rtti(); }                     \
    virtual const RTTI* get_rtti() const modifier;                                                                  \
    virtual const void* cast_to(const RTTI* rtti) const modifier;                                                   \
    static void create_rtti(RTTI& rtti);                                                                            \

# define DECLARE_RTTI_BASE(linkage, class_name) DECLARE_RTTI(linkage, class_name, )

# define DECLARE_RTTI_DERIVED(linkage, class_name) DECLARE_RTTI(linkage, class_name, override)

#define IMPLEMENT_RTTI(class_name)                                                                                  \
    RTTI* get_rtti_of_type(class_name*)                                                                             \
    {                                                                                                               \
        static RTTI rtti(#class_name, sizeof(class_name), []() -> void * { return new class_name; }, [](void* object) { delete (class_name*)object; }, &class_name::create_rtti); \
        return &rtti;                                                                                               \
    }                                                                                                               \
    const RTTI* class_name::get_rtti() const                                                                        \
    {                                                                                                               \
        return get_rtti_of_type((class_name*)nullptr);                                                              \
    }                                                                                                               \
    const void* class_name::cast_to(const RTTI* rtti) const                                                         \
    {                                                                                                               \
        return get_rtti_of_type((class_name*)nullptr)->cast_to(rtti, (const void*)this);                            \
    }                                                                                                               \
    void class_name::create_rtti(RTTI& rtti)                                                                        \

#define DECLARE_RTTI_OUTSIDE(linkage, class_name)                                                                   \
public:                                                                                                             \
    friend linkage RTTI* get_rtti_of_type(class_name*);                                                             \
    friend inline const RTTI* get_rtti(const class_name* object) { return get_rtti_of_type((class_name*)nullptr); } \
    void create_rtti_##class_name(RTTI& rtti);                                                                      \

#define IMPLEMENT_RTTI_OUTSIDE(class_name)                                                                          \
    RTTI* get_rtti_of_type(class_name*)                                                                             \
    {                                                                                                               \
        static RTTI rtti(#class_name, sizeof(class_name), []() -> void * { return new class_name; }, [](void* object) { delete (class_name*)object; }, &create_rtti_##class_name); \
        return &rtti;                                                                                               \
    }                                                                                                               \
    void create_rtti_##class_name(RTTI& rtti)                                                                       \

#define BASE_CLASS_OFFSET(class_name, base_class_name) ((int(uint64_t((base_class_name*)((class_name*)0x10000))))-0x10000)

#define ADD_BASE_CLASS(class_name, base_class_name) rtti.add_base_class(FIND_RTTI(base_class_name), BASE_CLASS_OFFSET(class_name, base_class_name));

template <class Type>
inline bool is_type(const Type* object, const RTTI* rtti)
{
    return object == nullptr || *object->get_rtti() == *rtti;
}

template <class Type>
inline bool is_kind_of(const Type* object, const RTTI* rtti)
{
    return object == nullptr || object->get_rtti()->is_kind_of(rtti);
}