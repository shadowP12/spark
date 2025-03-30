#pragma once

#include <vector>
#include <unordered_map>

#define INVALID_OBJECT_S 0

template<typename T>
class ObjectPool
{
public:
    ObjectPool(uint32_t reserved_count = 0)
    {
        _objects.reserve(reserved_count);
        _lookup.reserve(reserved_count);
    }

    void clear()
    {
        _objects.clear();
        _lookup.clear();
    }

    size_t size() const { return _objects.size(); }

    T* data() { return _objects.data(); }

    uint32_t add()
    {
        static uint32_t next = 0;
        next++;

        _lookup[next] = _objects.size();
        _objects.emplace_back();

        return next;
    }

    void remove(uint32_t handle)
    {
        auto it = _lookup.find(handle);
        if (it != _lookup.end())
        {
            const uint32_t index = it->second;

            if (index < _objects.size() - 1)
            {
                _objects[index] = std::move(_objects.back());
                _lookup[_objects.size()] = index;
            }

            _objects.pop_back();
            _lookup.erase(handle);
        }
    }

    T* get_obj(uint32_t handle)
    {
        auto it = _lookup.find(handle);
        if (it != _lookup.end())
        {
            return &_objects[it->second];
        }
        return nullptr;
    }

    const T* get_obj(uint32_t handle) const
    {
        const auto it = _lookup.find(handle);
        if (it != _lookup.end())
        {
            return &_objects[it->second];
        }
        return nullptr;
    }

    T* operator[](size_t index) { return &_objects[index]; }

    const T* operator[](size_t index) const { return &_objects[index]; }

private:
    std::vector<T> _objects;
    std::unordered_map<uint32_t, uint32_t> _lookup;
};