#ifndef SPLAY_H
#define SPLAY_H

#include <vector>
#include <string>
#include <iostream>

class SplayTree {
private:
    struct Node {
        long long key;
        Node* left;
        Node* right;
        Node* parent;
        Node(long long key);
        ~Node();
    };
    
    Node* root;
    void leftRotate(Node* x);
    void rightRotate(Node* x);
    void splay(Node* x);
    Node* find(long long key);
    Node* findMin(Node* x);
    Node* findMax(Node* x);

public:
    SplayTree();
    ~SplayTree();
    void insert(long long key);
    void remove(long long key);
    bool exists(long long key);
    std::string next(long long key);
    std::string prev(long long key);
};

std::vector<std::string> get_ans();

#endif // SPLAY_H
