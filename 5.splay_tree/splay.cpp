#include "splay.h"

SplayTree::Node::Node(long long key) : key(key), left(nullptr), right(nullptr), parent(nullptr) {}

SplayTree::SplayTree() : root(nullptr) {}

SplayTree::Node::~Node() {
    delete left;
    delete right;
}

SplayTree::~SplayTree() {
    delete root;
}

void SplayTree::leftRotate(Node* parent) {
    Node* child = parent->right;
    if (child) {
        parent->right = child->left;
        if (child->left) {
            child->left->parent = parent;
        }
        child->parent = parent->parent;
    }

    if (!parent->parent) {
        root = child;
    } else if (parent == parent->parent->left) {
        parent->parent->left = child;
    } else {
        parent->parent->right = child;
    }

    if (child) {
        child->left = parent;
    }
    parent->parent = child;
}

void SplayTree::rightRotate(Node* parent) {
    Node* child = parent->left;
    if (child) {
        parent->left = child->right;
        if (child->right) {
            child->right->parent = parent;
        }
        child->parent = parent->parent;
    }

    if (!parent->parent) {
        root = child;
    } else if (parent == parent->parent->left) {
        parent->parent->left = child;
    } else {
        parent->parent->right = child;
    }

    if (child) {
        child->right = parent;
    }
    parent->parent = child;
}

void SplayTree::splay(Node* node) {
    while (node && node->parent) {
        if (!node->parent->parent) {
            if (node == node->parent->left) {
                rightRotate(node->parent);
            } else {
                leftRotate(node->parent);
            }
        } else if (node == node->parent->left && node->parent == node->parent->parent->left) {
            rightRotate(node->parent->parent);
            rightRotate(node->parent);
        } else if (node == node->parent->right && node->parent == node->parent->parent->right) {
            leftRotate(node->parent->parent);
            leftRotate(node->parent);
        } else if (node == node->parent->right && node->parent == node->parent->parent->left) {
            leftRotate(node->parent);
            rightRotate(node->parent);
        } else {
            rightRotate(node->parent);
            leftRotate(node->parent);
        }
    }
}


SplayTree::Node* SplayTree::find(long long key) {
    Node* current = root;
    Node* parent = nullptr;
    while (current) {
        parent = current;
        if (key == current->key) {
            splay(current);
            return current;
        } else if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    
    if (parent) {
        splay(parent);
    }
    return nullptr;
}


SplayTree::Node* SplayTree::findMin(Node* node) {
    if (!node) {
        return nullptr;
    }
    while (node->left) {
        node = node->left;
    }
    splay(node);
    return node;
}


SplayTree::Node* SplayTree::findMax(Node* node) {
    if (!node) {
        return nullptr;
    }
    while (node->right) {
        node = node->right;
    }
    splay(node);
    return node;
}

void SplayTree::insert(long long key) {
    if (!root) {
        root = new Node(key);
        return;
    }
    Node* found = find(key);
    if (found && found->key == key) {
        return;
    }
    Node* newNode = new Node(key);
    if (!root) {
        root = newNode;
        return;
    }
    if (key < root->key) {
        newNode->left = root->left;
        if (root->left) {
            root->left->parent = newNode;
        }
        newNode->right = root;
        root->left = nullptr;
        root->parent = newNode;
    } else {
        newNode->right = root->right;
        if (root->right) {
            root->right->parent = newNode;
        }
        newNode->left = root;
        root->right = nullptr;
        root->parent = newNode;
    }
    root = newNode;
}

void SplayTree::remove(long long key) {
    Node* found = find(key);
    if (!found || found->key != key) {
        return;
    }
    Node* leftSubtree = found->left;
    if (leftSubtree) {
        leftSubtree->parent = nullptr;
    }
    Node* rightSubtree = found->right;
    if (rightSubtree) {
        rightSubtree->parent = nullptr;
    }
    found->left = nullptr;
    found->right = nullptr;
    delete found;
    if (!leftSubtree) {
        root = rightSubtree;
    } else if (!rightSubtree) {
        root = leftSubtree;
    } else {
        Node* maxLeft = findMax(leftSubtree);
        maxLeft->right = rightSubtree;
        rightSubtree->parent = maxLeft;
        root = maxLeft;
    }
}

bool SplayTree::exists(long long key) {
    Node* found = find(key);
    return found && found->key == key;
}

std::string SplayTree::next(long long key) {
    if (!root) {
        return "none";
    }
    Node* successor = nullptr;
    Node* current = root;
    
    while (current) {
        if (current->key > key) {
            successor = current;
            current = current->left;
        } else {
            current = current->right;
        }
    }
    if (successor) {
        splay(successor);
        return std::to_string(successor->key);
    }
    return "none";
}

std::string SplayTree::prev(long long key) {
    if (!root) {
        return "none";
    }
    Node* predecessor = nullptr;
    Node* current = root;
    
    while (current) {
        if (current->key < key) {
            predecessor = current;
            current = current->right;
        } else {
            current = current->left;
        }
    }
    if (predecessor) {
        splay(predecessor);
        return std::to_string(predecessor->key);
    }
    return "none";
}