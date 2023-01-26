#pragma once

#include "sp_event.h"

namespace sp
{
class Application
{
public:
    static Application* Get()
    {
        static Application _instance;
        return &_instance;
    }

    void Initialize();

    void Terminate();

    void Tick();
};
}// namespace sp