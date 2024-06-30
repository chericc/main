#include "tree2dot.hpp"

#include <list>
#include <sstream>

using namespace Tree;

struct NodeInfo {};

struct EdgeInfo {
    std::string namea;
    std::string nameb;
    std::string labela;
    std::string labelb;
};

void parsebtree(const BTreeNode *subtree, std::list<NodeInfo> nodes) {
    NodeInfo ni;
    ni.namea = subtree->;
    nodes.push_back(subtree->);
}

std::string btree2dot(const BTreeNode *node) { std::stringstream ss; }