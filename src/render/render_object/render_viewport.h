#pragma once

#include "core/unique_id.h"
#include "math/math_define.h"
#include "rhi/ez_vulkan.h"
#include <unordered_map>

class RenderViewport
{
public:
    RenderViewport();

    ~RenderViewport();

    void set_size(int w, int h);

private:
    void _update_render_target();

    void _clear_render_target() const;

public:
    int width = 0;
    int height = 0;
    EzTexture color = nullptr;
    EzTexture depth = nullptr;
};