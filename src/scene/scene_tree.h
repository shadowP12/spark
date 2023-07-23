#pragma once

#include "core/module.h"

class Node;

class SceneTree : public Module<SceneTree>
{
public:
    SceneTree();

    ~SceneTree();

    Node* get_root() { return _root; }

private:
    Node* _root = nullptr;
};