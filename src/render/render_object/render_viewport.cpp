#include "render_viewport.h"

RenderViewport::RenderViewport()
{
}

RenderViewport::~RenderViewport()
{
}

void RenderViewport::set_size(int w, int h)
{
    width = w;
    height = h;
    _update_render_target();
}

void RenderViewport::_update_render_target()
{
    _clear_render_target();

    if (width <= 0 || height <= 0)
        return;

    EzTextureDesc desc{};
    desc.width = width;
    desc.height = height;
    desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    desc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ez_create_texture(desc, color);
    ez_create_texture_view(color, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);

    desc.format = VK_FORMAT_D24_UNORM_S8_UINT;
    desc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ez_create_texture(desc, depth);
    ez_create_texture_view(depth, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
}

void RenderViewport::_clear_render_target() const
{
    if (color)
        ez_destroy_texture(color);

    if (depth)
        ez_destroy_texture(depth);
}