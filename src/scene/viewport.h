#pragma once

#include "node.h"

class RenderViewport;

class Viewport : public Node
{
public:
    Viewport();

    ~Viewport();

private:
    RenderViewport* _render_viewport = nullptr;
};