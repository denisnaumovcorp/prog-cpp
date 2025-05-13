#include "splay.h"

int main() {
    SplayTree tree;
    long long n;
    std::cin >> n;
    std::string command;
    long long value;
    for (int i = 0; i < n; i++) {
        std::cin >> command >> value;   
        if (command == "insert") {
            tree.insert(value);
        } else if (command == "delete") {
            tree.remove(value);
        } else if (command == "exists") {
            std::cout << (tree.exists(value) ? "true" : "false") << "\n";
        } else if (command == "next") {
            std::cout << tree.next(value) << "\n";
        } else if (command == "prev") {
            std::cout << tree.prev(value) << "\n";
        }
    }
}
