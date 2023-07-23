#include "scene_tree.h"
#include "node.h"

SceneTree::SceneTree()
{
    _root = new Node();
    _root->_set_tree(this);
}

SceneTree::~SceneTree()
{
    delete _root;
}