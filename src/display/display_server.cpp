#include "display_server.h"
#include "render/render_system.h"
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif

void DisplaySystem::setup()
{
    // Main window
    WindowID main_window = _create_window();
}

void DisplaySystem::cleanup()
{
    for (auto window_iter : _windows)
    {
        RenderSystem::get()->destroy_window(window_iter.first);
        glfwDestroyWindow(window_iter.second);
    }
    _windows.clear();
}

DisplaySystem::WindowID DisplaySystem::_create_window()
{
    WindowID id = _next_window;
    _next_window++;

    GLFWwindow* glfw_window = glfwCreateWindow(800, 600, "spark", nullptr, nullptr);
    RenderSystem::get()->create_window(id, glfwGetWin32Window(glfw_window));
    _windows[id] = glfw_window;

    return id;
}

bool DisplaySystem::should_close()
{
    if (glfwWindowShouldClose(_windows[MAIN_WINDOW_ID]))
    {
        return true;
    }
    return false;
}