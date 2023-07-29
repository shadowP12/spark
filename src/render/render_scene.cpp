#include "render_scene.h"

RenderScene::RenderScene()
{

}

RenderScene::~RenderScene()
{

}

void RenderScene::create_window(int window_id, void* window)
{
    EzSwapchain swapchain = VK_NULL_HANDLE;
    ez_create_swapchain(window, swapchain);
    swapchains[window_id] = swapchain;
}

void RenderScene::destroy_window(int window_id)
{
    auto iter = swapchains.find(window_id);
    if (iter != swapchains.end())
    {
        ez_destroy_swapchain(iter->second);
        swapchains.erase(iter);
    }
}