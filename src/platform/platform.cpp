#include "platform.h"
#include "display/display_server.h"
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif

void Platform::setup()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    DisplaySystem::get()->setup();
}

void Platform::cleanup()
{
    DisplaySystem::get()->cleanup();
    glfwTerminate();
}

double Platform::get_time()
{
    return glfwGetTime();
}

bool Platform::poll_events()
{
    if (DisplaySystem::get()->should_close())
    {
        return false;
    }

    glfwPollEvents();
    return true;
}
