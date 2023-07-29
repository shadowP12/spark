#pragma once

#include "core/module.h"
#include "core/event.h"
#include <map>

class RenderScene;

class RenderSystem : public Module<RenderSystem>
{
public:
    RenderSystem();

    ~RenderSystem();

    void setup() {}

    void cleanup() {}

    void create_window(int window_id, void* window);

    void destroy_window(int window_id);

private:
    RenderScene* _scene;
};