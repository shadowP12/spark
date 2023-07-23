#include "node.h"
#include "scene_tree.h"

Node::Node()
{
}

Node::~Node()
{
    if (_parent)
    {
        _parent->remove_child(this);
    }

    while (_children.size())
    {
        Node* child = _children[_children.size() - 1];
        remove_child(child);
        delete(child);
    }
}

void Node::add_child(Node* child)
{
    _children.push_back(child);
    child->_parent = this;

    if (_tree)
    {
        child->_set_tree(_tree);
    }
}

void Node::remove_child(Node* child)
{
    for (auto iter = _children.begin(); iter != _children.end(); )
    {
        if (iter == _children.end())
            break;

        if (*iter == child) {
            _children.erase(iter);
            child->_parent = nullptr;
            child->_set_tree(nullptr);
        }
    }
}

void Node::_set_tree(SceneTree* tree)
{
    _tree = tree;
}