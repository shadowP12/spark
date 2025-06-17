#pragma once

#include "math/bounding_box.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>

#include <functional>
#include <stack>
#include <string>

template<typename>
constexpr bool always_false = false;

class SerializationContext
{
public:
    SerializationContext(
        rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer = nullptr) : _writer(writer) {}

    void object(std::function<void()> func)
    {
        _writer->StartObject();
        func();
        _writer->EndObject();
    }

    void array(const char* key, std::function<void()> func)
    {
        _writer->Key(key);
        _writer->StartArray();
        func();
        _writer->EndArray();
    }

    void key(const char* key)
    {
        _writer->Key(key);
    }

    template<typename T>
    void field(const char* key, T val)
    {
        _writer->Key(key);
        field(val);
    }

    template<typename T>
    void field(T val)
    {
        using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;

        if constexpr (std::is_integral_v<ValueType>) _writer->Int(val);
        else if constexpr (std::is_same_v<ValueType, float>)
            _writer->Double(val);
        else if constexpr (std::is_same_v<ValueType, double>)
            _writer->Double(val);
        else if constexpr (std::is_same_v<ValueType, bool>)
            _writer->Bool(val);
        else if constexpr (std::is_same_v<ValueType, std::string>)
            _writer->String(val.c_str());
        else if constexpr (std::is_same_v<ValueType, glm::vec2>)
        {
            _writer->StartArray();
            _writer->Double(val[0]);
            _writer->Double(val[1]);
            _writer->EndArray();
        }
        else if constexpr (std::is_same_v<ValueType, glm::vec3>)
        {
            _writer->StartArray();
            _writer->Double(val[0]);
            _writer->Double(val[1]);
            _writer->Double(val[2]);
            _writer->EndArray();
        }
        else if constexpr (std::is_same_v<ValueType, glm::vec4>)
        {
            _writer->StartArray();
            _writer->Double(val[0]);
            _writer->Double(val[1]);
            _writer->Double(val[2]);
            _writer->Double(val[3]);
            _writer->EndArray();
        }
        else if constexpr (std::is_same_v<ValueType, BoundingBox>)
        {
            _writer->StartArray();
            _writer->Double(val.bb_min.x);
            _writer->Double(val.bb_min.y);
            _writer->Double(val.bb_min.z);
            _writer->Double(val.bb_max.x);
            _writer->Double(val.bb_max.y);
            _writer->Double(val.bb_max.z);
            _writer->EndArray();
        }
        else if constexpr (std::is_enum_v<ValueType>)
        {
            _writer->Int(static_cast<std::underlying_type_t<ValueType>>(val));
        }
        else
            static_assert(always_false<ValueType>, "Unsupported type");
    }

private:
    rapidjson::PrettyWriter<rapidjson::StringBuffer>* _writer;
};

class DeserializationContext
{
public:
    DeserializationContext(const rapidjson::Value* root = nullptr) : _current(root) {}

    bool has_key(const char* key) const
    {
        return _current->HasMember(key);
    }

    void object(const char* key, std::function<void()> func)
    {
        if (_current && _current->HasMember(key))
        {
            const auto& obj = (*_current)[key];
            if (obj.IsObject())
            {
                _stack.push(_current);
                _current = &obj;
                func();
                _current = _stack.top();
                _stack.pop();
            }
        }
    }

    void array(const char* key, std::function<void()> item_processor)
    {
        if (_current->HasMember(key))
        {
            const auto& array = (*_current)[key];
            if (array.IsArray())
            {
                for (const auto& item : array.GetArray())
                {
                    _stack.push(_current);
                    _current = &item;
                    item_processor();
                    _current = _stack.top();
                    _stack.pop();
                }
            }
        }
    }

    template<typename T>
    bool field(const char* key, T& out_value) const
    {
        if (!_current || !_current->HasMember(key))
        {
            return false;
        }
        return field((*_current)[key], out_value);
    }

    template<typename T>
    bool field(int key, T& out_value) const
    {
        if (!_current || !_current->IsArray() || key >= _current->Size())
        {
            return false;
        }
        return field((*_current)[key], out_value);
    }

    template<typename T>
    bool field(const rapidjson::Value& value, T& out_value) const
    {
        if constexpr (std::is_integral_v<T>)
        {
            if (value.IsInt())
            {
                out_value = value.GetInt();
                return true;
            }
        }
        else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
        {
            if (value.IsNumber())
            {
                out_value = value.GetDouble();
                return true;
            }
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            if (value.IsBool())
            {
                out_value = value.GetBool();
                return true;
            }
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            if (value.IsString())
            {
                out_value = value.GetString();
                return true;
            }
        }
        else if constexpr (std::is_same_v<T, glm::vec2>)
        {
            if (value.IsArray())
            {
                out_value = glm::vec2(value[0].GetDouble(), value[1].GetDouble());
                return true;
            }
        }
        else if constexpr (std::is_same_v<T, glm::vec3>)
        {
            if (value.IsArray())
            {
                out_value = glm::vec3(value[0].GetDouble(), value[1].GetDouble(), value[2].GetDouble());
                return true;
            }
        }
        else if constexpr (std::is_same_v<T, glm::vec4>)
        {
            if (value.IsArray())
            {
                out_value = glm::vec4(value[0].GetDouble(), value[1].GetDouble(), value[2].GetDouble(), value[3].GetDouble());
                return true;
            }
        }
        else if constexpr (std::is_same_v<T, BoundingBox>)
        {
            if (value.IsArray())
            {
                out_value.bb_min = glm::vec3(value[0].GetDouble(), value[1].GetDouble(), value[2].GetDouble());
                out_value.bb_max = glm::vec3(value[4].GetDouble(), value[4].GetDouble(), value[5].GetDouble());
                return true;
            }
        }
        else if constexpr (std::is_enum_v<T>)
        {
            if (value.IsInt())
            {
                out_value = static_cast<T>(value.GetInt());
                return true;
            }
        }
        return false;
    }

private:
    const rapidjson::Value* _current;
    std::stack<const rapidjson::Value*> _stack;
};