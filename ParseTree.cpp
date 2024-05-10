#include <iostream>
#include <vector>
#include <string>
using namespace std;

class ParseTreeNode {
public:
    string type;
    vector<ParseTreeNode*> children;
    bool isTerminal = false;

    ParseTreeNode(string type) : type(type) {}

    void addChild(ParseTreeNode* child) {
        children.push_back(child);
    }

    vector<ParseTreeNode*> getChildren() {
        return children;
    }

    string getType() {
        return type;
    }

    void printTree(int level = 0) {
    // Only print nodes that have children
    if (!children.empty()) {
        // Print indentation
        for (int i = 0; i < level; i++) {
            std::cout << "  ";
        }

        // Print node type
        std::cout << type;

        // Print children
        std::cout << " -> { ";
        for (size_t i = 0; i < children.size(); i++) {
            if(!children[i]->isTerminal && children[i]->children.empty()){
                continue;
            }
            std::cout << children[i]->type;
            if (i != children.size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << " }";

        std::cout << std::endl;

        // Recursively print children's trees
        for (ParseTreeNode* child : children) {
            child->printTree(level + 1);
        }
    }
}
};