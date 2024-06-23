#include "bst.h"

#include <iostream>

TreeNode::TreeNode() : key(), lnode(nullptr), rnode(nullptr), parent(nullptr) {}

TreeNode::TreeNode(const KEY_TYPE& _key)
    : key(_key), lnode(nullptr), rnode(nullptr), parent(nullptr) {}

TreeNode::TreeNode(const KEY_TYPE& _key, TreeNode* _lnode, TreeNode* _rnode,
                   TreeNode* _parent)
    : key(_key), lnode(_lnode), rnode(_rnode), parent(_parent) {}

BSTree::BSTree() { m_size = 0; }

BSTree::~BSTree() { free_tree__(m_rootnode.rnode); }

bool BSTree::empty() const { return m_size == 0; }

std::size_t BSTree::size() const { return m_size; }

void BSTree::clear() {
    free_tree__(m_rootnode.rnode);
    m_rootnode.rnode = nullptr;
    m_size = 0;

    return;
}

KEY_TYPE* BSTree::insert(const KEY_TYPE& key) {
    TreeNode* node = locate__(key);

    if (nullptr == node) {
        m_rootnode.rnode = new TreeNode(key, nullptr, nullptr, &m_rootnode);
        ++m_size;
        return &m_rootnode.rnode->key;
    } else {
        if (node->key == key) {
            return nullptr;
        } else if (key < node->key) {
            node->lnode = new TreeNode(key, nullptr, nullptr, node);
            ++m_size;
            return &node->lnode->key;
        } else {
            node->rnode = new TreeNode(key, nullptr, nullptr, node);
            ++m_size;
            return &node->rnode->key;
        }
    }

    return nullptr;
}

void BSTree::erase(const KEY_TYPE& key) {
    TreeNode* node = locate__(key);
    erase_node__(node);
    return;
}

KEY_TYPE* BSTree::find(const KEY_TYPE& key) const {
    TreeNode* node = locate__(key);
    if (nullptr == node || node->key != key) {
        return nullptr;
    } else {
        return &node->key;
    }
    return nullptr;
}

void BSTree::preorder_print() const {
    std::cout << "preorder:" << std::endl;
    preorder_print__(m_rootnode.rnode);
    std::cout << std::endl;

    return;
}

void BSTree::inorder_print() const {
    std::cout << "inorder:" << std::endl;
    inorder_print__(m_rootnode.rnode);
    std::cout << std::endl;

    return;
}

std::size_t BSTree::height() const { return max_depth__(m_rootnode.rnode); }

void BSTree::free_tree__(TreeNode* node) {
    if (nullptr == node) {
        return;
    }

    if (node->lnode != nullptr) {
        free_tree__(node->lnode);
    }

    if (node->rnode != nullptr) {
        free_tree__(node->rnode);
    }

    delete node;

    return;
}

TreeNode* BSTree::locate__(const KEY_TYPE& key) const {
    TreeNode* node = m_rootnode.rnode;

    while (nullptr != node) {
        if (node->key == key) {
            break;
        } else if (node->key > key) {
            if (node->lnode != nullptr) {
                node = node->lnode;
            } else {
                break;
            }
        } else {
            if (node->rnode != nullptr) {
                node = node->rnode;
            } else {
                break;
            }
        }
    }

    return node;
}

void BSTree::preorder_print__(const TreeNode* node) const {
    if (nullptr == node) {
        return;
    }

    std::cout << node->key << " ";
    preorder_print__(node->lnode);
    preorder_print__(node->rnode);

    return;
}

void BSTree::inorder_print__(const TreeNode* node) const {
    if (nullptr == node) {
        return;
    }

    inorder_print__(node->lnode);
    std::cout << node->key << " ";
    inorder_print__(node->rnode);

    return;
}

TreeNode* BSTree::predecessor__(TreeNode* node) const {
    if (nullptr == node) {
        return nullptr;
    }

    TreeNode* nodeit = node;

    if (nodeit->lnode != nullptr) {
        nodeit = nodeit->lnode;

        while (nodeit->rnode != nullptr) {
            nodeit = nodeit->rnode;
        }

        return nodeit;
    } else {
        while (nodeit->parent != &m_rootnode) {
            if (nodeit->parent->rnode == nodeit) {
                return nodeit->parent;
            }
            nodeit = nodeit->parent;
        }
    }

    if (nodeit == node) {
        return nullptr;
    } else {
        return nodeit;
    }
}

TreeNode* BSTree::successor__(TreeNode* node) const {
    if (nullptr == node) {
        return nullptr;
    }

    TreeNode* nodeit = node;

    if (nodeit->rnode != nullptr) {
        nodeit = nodeit->rnode;

        while (nodeit->lnode != nullptr) {
            nodeit = nodeit->lnode;
        }
    } else {
        while (nodeit->parent != &m_rootnode) {
            if (nodeit->parent->lnode == nodeit) {
                return nodeit->parent;
            }
            nodeit = nodeit->parent;
        }
    }

    if (nodeit == node) {
        return nullptr;
    } else {
        return nodeit;
    }
}

void BSTree::erase_node__(TreeNode* node) {
    if (nullptr == node) {
        return;
    }

    if (nullptr == node->lnode && nullptr == node->rnode) {
        /**
         * If node has no child, then just delete it.
         * Only one pointer needs to change.
         */

        if (node->parent != nullptr) {
            if (node->parent->lnode == node) {
                node->parent->lnode = nullptr;
            } else if (node->parent->rnode == node) {
                node->parent->rnode = nullptr;
            }
        }

        delete node;
        node = nullptr;
        --m_size;
    } else if (nullptr == node->lnode && nullptr != node->rnode) {
        /**
         * If node has only one child, then just replace node
         * with its only child.
         * Two pointers need to change.
         */
        if (node->parent != nullptr) {
            if (node->parent->lnode == node) {
                node->parent->lnode = node->rnode;
            } else if (node->parent->rnode == node) {
                node->parent->rnode = node->rnode;
            }
        }
        node->rnode->parent = node->parent;

        delete node;
        node = nullptr;
        --m_size;
    } else if (nullptr == node->rnode && nullptr != node->lnode) {
        /**
         * Same with previous condition.
         */
        if (nullptr != node->parent) {
            if (node->parent->lnode == node) {
                node->parent->lnode = node->lnode;
            } else if (node->parent->rnode == node) {
                node->parent->rnode = node->lnode;
            }
        }
        node->lnode->parent = node->parent;

        delete node;
        node = nullptr;
        --m_size;
    } else {
        /* If the node have both left child and right child. */

        TreeNode* successor = successor__(node);

        if (node->rnode == successor) {
            /**
             * If the node's right child is the successor, ie,
             * the node's right child has no left child.
             * Then we need just to replace node with its right
             * child.
             * Four pointers need to change:
             * n.p.l/r, n.l.p, n.r.p, n.r.l
             */

            if (nullptr != node->parent) {
                if (node->parent->lnode == node) {
                    node->parent->lnode = node->rnode;
                } else if (node->parent->rnode == node) {
                    node->parent->rnode = node->rnode;
                }
            }

            node->lnode->parent = node->rnode;
            node->rnode->parent = node->parent;
            node->rnode->lnode = node->lnode;
        } else {
            /**
             * If the node's right is not successor, ie,
             * the node's right child has left child, and
             * the node's succssor is the far left node of
             * the node's left tree.
             * First, use the succssor's right child to replace
             * the succssor. Two pointers will be changed.
             * Second, use the succssor to replace the node.
             * Six pointers need to change.
             */

            /* First step. */
            /* successor MUST be its parent's left child. */
            successor->parent->lnode = successor->rnode;
            if (successor->rnode != nullptr) {
                successor->rnode->parent = successor->parent;
            }

            /* Second step. */
            if (nullptr != node->parent) {
                if (node->parent->lnode == node) {
                    node->parent->lnode = successor;
                } else if (node->parent->rnode == node) {
                    node->parent->rnode = successor;
                }
            }

            node->lnode->parent = successor;
            node->rnode->parent = successor;

            successor->parent = node->parent;
            successor->lnode = node->lnode;
            successor->rnode = node->rnode;
        }

        delete node;
        node = nullptr;
        --m_size;
    }

    return;
}

std::size_t BSTree::max_depth__(TreeNode* node) const {
    std::size_t depth = 0;

    if (nullptr != node) {
        if (nullptr != node->lnode) {
            std::size_t depth_tmp = max_depth__(node->lnode);
            if (depth_tmp > depth) {
                depth = depth_tmp;
            }
        }

        if (nullptr != node->rnode) {
            std::size_t depth_tmp = max_depth__(node->rnode);
            if (depth_tmp > depth) {
                depth = depth_tmp;
            }
        }

        depth += 1;
    }

    return depth;
}