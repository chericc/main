
#include <array>

#include "alg.hpp"
#include "printtree.hpp"
#include "treenode.hpp"
#include "xlog.hpp"

namespace Tree {

class TestBTreeNode : public BTreeNode {
   public:
    ~TestBTreeNode() override {}
    BTreeNode *parent() override { return parent_; }
    void set_parent(BTreeNode *node) override { parent_ = node; }
    BTreeNode *left() override { return left_; }
    void set_left(BTreeNode *node) override { left_ = node; }
    BTreeNode *right() override { return right_; }
    void set_right(BTreeNode *node) override { right_ = node; }
    value_t const &value() override { return value_; }
    void set_value(value_t const &value) override { value_ = value; }
    std::string to_text() override {
        if (parent_) {
            return std::to_string(value_) + "(" +
                   std::to_string(parent_->value()) + ")";
        } else {
            return std::to_string(value_);
        }
    }

    value_t value_ = 0;
    BTreeNode *left_ = nullptr;
    BTreeNode *right_ = nullptr;
    BTreeNode *parent_ = nullptr;
};

void printtreedemo_registertest() {
    auto test = [&]() {
        std::array<BTreeNode *, 5> nodes;
        for (int i = 0; i < (int)nodes.size(); ++i) {
            nodes[i] = new TestBTreeNode();
            nodes[i]->set_value(i);
        }
        nodes[0]->set_left(nodes[1]);
        nodes[0]->set_right(nodes[2]);
        nodes[1]->set_left(nodes[3]);
        nodes[2]->set_left(nodes[4]);

        std::string str = print_tree(nodes[0], nullptr);
        xlog_dbg("tree is: \n%s\n", str.c_str());
        for (auto &ref : nodes) {
            delete ref;
        }
    };

    MainAlgManager::Funcs funcs;
    funcs["base"] = test;
    MainAlgManager::getInstance().add("printtreedemo", funcs);
}

}  // namespace Tree
