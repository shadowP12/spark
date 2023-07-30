#pragma once

#include "core/module.h"
#include "core/event.h"
#include "rhi/ez_vulkan.h"
#include <map>

class RenderScene;
class RenderCanvas;
class RenderStorage;
class RenderViewport;

class RenderSystem : public Module<RenderSystem>
{
public:
    RenderSystem();

    ~RenderSystem();

    void setup() {}

    void cleanup() {}

    void draw();

    void create_window(int window_id, void* window);

    void destroy_window(int window_id);

    RenderViewport* create_viewport();

    void destroy_viewport(RenderViewport* viewport);

public:
    std::map<int, EzSwapchain> swapchains;
    std::vector<RenderViewport*> viewports;
};

class RenderSystemGlobals
{
public:
    static RenderStorage* storage;
    static RenderCanvas* canvas;
    static RenderScene* scene;
};

#define RSG RenderSystemGlobals