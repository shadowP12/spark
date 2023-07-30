#include "viewport.h"
#include "render/render_system.h"
#include "render/render_object/render_viewport.h"

Viewport::Viewport()
{
    _render_viewport = RenderSystem::get()->create_viewport();
}

Viewport::~Viewport()
{
    RenderSystem::get()->destroy_viewport(_render_viewport);
    _render_viewport = nullptr;
}

