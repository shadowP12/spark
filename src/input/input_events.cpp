#include "input_events.h"

Event<MouseEvent> g_mouse_event;
Event<KeyboardEvent> g_keyboard_event;

Event<MouseEvent>& Input::get_mouse_event()
{
    return g_mouse_event;
}

Event<KeyboardEvent>& Input::get_keyboard_event()
{
    return g_keyboard_event;
}