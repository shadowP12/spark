#include "render_system.h"
#include "render_scene.h"
#include "rhi/ez_vulkan.h"
#include "rhi/shader_manager.h"
#include "rhi/shader_compiler.h"

RenderSystem::RenderSystem()
{
    ez_init();
    ShaderCompiler::get()->setup();
    ShaderManager::get()->setup();
    _scene = new RenderScene();
}

RenderSystem::~RenderSystem()
{
    delete _scene;
    ShaderManager::get()->cleanup();
    ShaderCompiler::get()->cleanup();
    ez_terminate();
}

void RenderSystem::create_window(int window_id, void* window)
{
    _scene->create_window(window_id, window);
}

void RenderSystem::destroy_window(int window_id)
{
    _scene->destroy_window(window_id);
}