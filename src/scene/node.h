#pragma once
#include <vector>

class SceneTree;

class Node
{
public:
    Node();

    ~Node();

    void add_child(Node* child);

    void remove_child(Node* child);

private:
    friend SceneTree;
    void _set_tree(SceneTree*);

    SceneTree* _tree = nullptr;
    Node* _parent = nullptr;
    std::vector<Node*> _children;
};