#pragma once

#include "core/event.h"

class MouseEvent {
public:
    MouseEvent() = default;
    enum class Type
    {
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

namespace Input {
    static Event<MouseEvent> mouse_event;
}