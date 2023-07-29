#pragma once

#include <unordered_map>

typedef uint64_t UniqueId;

class UniqueAllocBase
{
public:
    uint64_t gen_id()
    {
        return next_id++;
    }

    static uint64_t next_id;
};

template <class T>
class UniqueAlloc : public UniqueAllocBase
{
public:
    UniqueId alloc()
    {
        uint64_t id = gen_id();
        _dict[id] = new T;
        return id;
    }

    void free_item(const UniqueId& id)
    {
        auto iter = _dict.find(id);
        if (iter != _dict.end())
        {
            delete iter->second;
            _dict.erase(iter);
        }
    }

    void initialize(const UniqueId& id, const T& value)
    {
        T* mem = get(id);
        new (mem) T(value);
    }

    T* get(const UniqueId& id)
    {
        auto iter = _dict.find(id);
        if (iter != _dict.end())
        {
            return iter->second;
        }
        return nullptr;
    }

private:
    std::unordered_map<uint64_t, T*> _dict;
};