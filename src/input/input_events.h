#pragma once

#include "core/event.h"

class MouseEvent
{
public:
    MouseEvent() = default;
    enum class Type {
        DOWN,
        UP,
        MOVE,
        WHEEL,
        UNKNOWN
    };

    float x = 0.0f;
    float y = 0.0f;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    uint16_t button = 0;
    uint32_t window_id = 0;
    Type type = Type::UNKNOWN;
};

// Glfw keys
enum class KeyCode {
    ARROW_RIGHT = 262,
    ARROW_LEFT = 263,
    ARROW_DOWN = 264,
    ARROW_UP = 265,
};

class KeyboardEvent
{
public:
    KeyboardEvent() = default;
    enum class Action {
        RELEASE,
        PRESS,
        REPEAT,
        UNKNOWN
    };

    int key = -1;
    uint32_t window_id = 0;
    Action action = Action::UNKNOWN;
};

class Input
{
public:
    static Event<MouseEvent>& get_mouse_event();
    static Event<KeyboardEvent>& get_keyboard_event();
};