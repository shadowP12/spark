#pragma once

#include "rhi/ez_vulkan.h"
#include<map>

class RenderScene
{
public:
    RenderScene();

    ~RenderScene();

    void create_window(int window_id, void* window);

    void destroy_window(int window_id);

    std::map<int, EzSwapchain> swapchains;
};