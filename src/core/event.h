#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <vector>

#define EVENT_CB(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

typedef int EventHandle;

template<class... Args>
class Event
{
public:
    struct EventData
    {
        EventHandle handle = 0;
        std::function<void(Args...)> func;
    };

    EventHandle bind(std::function<void(Args...)> func)
    {
        EventData data;
        data.handle = ++_next_handle;
        data.func = func;
        _event_datas.push_back(data);
        return data.handle;
    }

    void unbind(EventHandle handle)
    {
        for (typename std::vector<EventData>::iterator iter = _event_datas.begin(); iter != _event_datas.end(); iter++)
        {
            EventData& data = *iter;
            if (data.handle == handle)
            {
                _event_datas.erase(iter);
                break;
            }
        }
    }

    void broadcast(Args... args)
    {
        if (_event_datas.size() <= 0)
            return;

        for (auto event_data : _event_datas)
            event_data.func(std::forward<Args>(args)...);
    }

private:
    EventHandle _next_handle = 0;
    std::vector<EventData> _event_datas;
};