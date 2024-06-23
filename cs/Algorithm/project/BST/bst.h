
/**
 * An example of BST implementation.
 * BST is short for Binary Search Tree.
 */

#pragma once

#include <cstddef>

typedef int KEY_TYPE;

class TreeNode {
   private:
    TreeNode();
    TreeNode(const KEY_TYPE& key);
    TreeNode(const KEY_TYPE& key, TreeNode* lnode, TreeNode* rnode,
             TreeNode* parent);

    KEY_TYPE key;
    TreeNode* lnode;
    TreeNode* rnode;
    TreeNode* parent;

    friend class BSTree;
};

class BSTree {
   public:
    BSTree();
    ~BSTree();
    bool empty() const;
    std::size_t size() const;
    void clear();
    KEY_TYPE* insert(const KEY_TYPE& key);
    void erase(const KEY_TYPE& key);
    KEY_TYPE* find(const KEY_TYPE& key) const;
    void preorder_print() const;
    void inorder_print() const;
    void predecessor_print() const;
    void successor_print() const;
    std::size_t height() const;

   private:
    void free_tree__(TreeNode* node);
    TreeNode* locate__(const KEY_TYPE& key) const;
    void preorder_print__(const TreeNode* node) const;
    void inorder_print__(const TreeNode* node) const;
    TreeNode* predecessor__(TreeNode* node) const;
    TreeNode* successor__(TreeNode* node) const;
    void erase_node__(TreeNode* node);
    std::size_t max_depth__(TreeNode* node) const;

   private:
    std::size_t m_size;

    /**
     * The root node.
     * This is a fake node.
     * Root node's right node will point to the real
     * root node.
     * This node is used to make the real root a
     * normal node.
     */
    TreeNode m_rootnode;
};
