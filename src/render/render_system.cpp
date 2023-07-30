#include "render_system.h"
#include "render_canvas.h"
#include "render_scene.h"
#include "render_storage.h"
#include "render_object/render_viewport.h"
#include "rhi/ez_vulkan.h"
#include "rhi/shader_compiler.h"
#include "rhi/shader_manager.h"

RenderSystem::RenderSystem()
{
    ShaderCompiler::get()->setup();
    ShaderManager::get()->setup();
    RSG::scene = new RenderScene();
    RSG::canvas = new RenderCanvas();
    RSG::storage = new RenderStorage();
}

RenderSystem::~RenderSystem()
{
    delete RSG::scene;
    delete RSG::canvas;
    delete RSG::storage;

    ShaderManager::get()->cleanup();
    ShaderCompiler::get()->cleanup();
}

void RenderSystem::create_window(int window_id, void* window)
{
    EzSwapchain swapchain = VK_NULL_HANDLE;
    ez_create_swapchain(window, swapchain);
    swapchains[window_id] = swapchain;
}

void RenderSystem::destroy_window(int window_id)
{
    auto iter = swapchains.find(window_id);
    if (iter != swapchains.end())
    {
        ez_destroy_swapchain(iter->second);
        swapchains.erase(iter);
    }
}

RenderViewport* RenderSystem::create_viewport()
{
    auto* viewport = new RenderViewport();
    viewports.push_back(viewport);
    return  viewport;
}

void RenderSystem::destroy_viewport(RenderViewport* viewport)
{
    for (auto iter = viewports.begin(); iter != viewports.end(); iter++)
    {
        if (*iter == viewport)
        {
            viewports.erase(iter);
            break;
        }
    }
}

void RenderSystem::draw()
{

}

RenderStorage* RenderSystemGlobals::storage = nullptr;
RenderCanvas* RenderSystemGlobals::canvas = nullptr;
RenderScene* RenderSystemGlobals::scene = nullptr;


