#pragma once

#include "core/module.h"
#include "core/event.h"
#include <map>

class GLFWwindow;

class DisplaySystem : public Module<DisplaySystem>
{
public:
    typedef int WindowID;

    enum
    {
        MAIN_WINDOW_ID = 0,
        INVALID_WINDOW_ID = -1
    };

    void setup();

    void cleanup();

    bool should_close();

private:
    WindowID _create_window();

    WindowID _next_window = MAIN_WINDOW_ID;
    std::map<WindowID, GLFWwindow*> _windows;
};