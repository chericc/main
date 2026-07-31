#include "rbtree.hpp"

#include "printtree.hpp"
#include "xlog.hpp"

namespace Tree {

RBTree::Color RBTree::color(BTreeNode *node)
{
    Color color = static_cast<RBTreeNode*>(node)->color();
    return color;
}

void RBTree::set_color(BTreeNode *node, Color color)
{
    static_cast<RBTreeNode*>(node)->set_color(color);
    return ;
}

RBTree::RBTree() {
    null_ = new RBTreeNode();
    null_->set_left(null_);
    null_->set_right(null_);
    null_->set_parent(null_);
    root_ = null_;
}

RBTree::~RBTree() 
{
    destroy(root_); 
    root_ = nullptr;
    delete null_;
    null_ = nullptr;
}

void RBTree::destroy(BTreeNode *node) {
    if (node != null_) {
        if (node->left() != null_) {
            destroy(node->left());
        }
        if (node->right() != null_) {
            destroy(node->right());
        }
        
        delete node;
    }
}

int RBTree::insert(value_t const& value)
{
    BTreeNode *node = new RBTreeNode;
    node->set_left(null_);
    node->set_right(null_);
    node->set_parent(null_);
    node->set_value(value);

    insert(node);
    return 0;
}

int RBTree::exist(value_t const& value)
{
    bool found_flag = false;

    do {
        if (root_ == null_) {
            break;
        }

        BTreeNode *node = root_;
        while (node != null_) {
            if (value < node->value()) {
                node = node->left();
            } else if (value > node->value()) {
                node = node->right();
            } else {
                found_flag = true;
                break;
            }
        }
    } while (0);

    return found_flag;     
}

void RBTree::clear()
{
    destroy(root_); 
    root_ = null_;
}

int RBTree::remove(BTreeNode *node)
{
/*
case 1: node has only one child.
replace node with its child. 
ynode points to node(to be removed). 
xnode points to child node(the new node).

if ynode is red, then ynode is not root. no rule will break.
if ynode is black, rule could break. try fix xnode.

case 2: node has two children.
replace node with its successor.
ynode points to its successor.
successor can be node's right child or not.
if successor is not node's right child, then replace successor 
with its right child(successor is free now). xnode points to 
successor's right child. then replace node with ynode. 
if successor is node's right child, replace node with it's right
child.

make node's color same as ynode's color.

node is replaced by the same color, so no rule could break.
xnode is actually removed. so xnode need fix.

if ynode is red, then xnode is not red. no rule will break.
if ynode is black, rule could break. try fix xnode.
*/

    BTreeNode *ynode = node;
    BTreeNode *xnode = ynode;

    Color color_y = color(ynode);

    if (node->left() == null_) {
        xnode = node->right();
        transplant(node, node->right());
    } else if (node->right() == null_) {
        xnode = node->left();
        transplant(node, node->left());
    } else {
        ynode = minimum(node->right());
        color_y = color(ynode);
        xnode = ynode->right();
        if (ynode->parent() != node) {
            transplant(xnode, ynode);
            ynode->set_right(node->right());
            ynode->right()->set_parent(ynode);
        }
        transplant(node, ynode);
        ynode->set_left(node->left());
        ynode->left()->set_parent(node->parent());
        set_color(ynode, color(node));
    }

    if (color_y == Black) {
        remove_fix(xnode);
    }
    return 0;
}

void RBTree::remove_fix(BTreeNode *node)
{
/*

in remove function, both cases have the following expectation:

1. only one node is removed and it's replaced by its child.

- in case 1, node is removed and it's replaced by its left or right child.
- in case 2, node is removed and it's replaced by its successor. successor
will have the same color as node, so no rule is broken. the successor is
then removed and it only has one child. so it becomes case 1.

2. xnode replaces the original black node and rule 5 is broke. we can
paint an additional black to xnode to fix this problem(thus rule 5 fixed and 
rule 1 broken).

if xnode's color is red or xnode is root, just paint it to black and remove 
its additional black. 

if xnode has two black color, we need to fix it.

-- how to fix rule 1(xnode now has two color)?

case 1: xnode's brother w is red.
w's children must be black node and xnode's parent node must be black.
make parent red and w black. left | right turn parent. xnode now has 
a black brother(w's children). then cases is case 2,3 or 4.

case 2: xnode's brother w is black, and both w's children is black.
remove black color from xnode and w, thus w's color is red. paint additional 
black to xnode's parent. xnode's parent tobe the new xnode.

case 3: like case 2, but w's x-side child is red.
rotate w in the off-x direction. make w's x-side node replacing w.
make w red and w's x-side node black. so it becomes case 4.

case 4: xnode's brother w is black, and w's off-x-side node is red.
turn xnode's parent and move xnode's additional black to its parent. 
keep xnode's parent's and w's original pos's color unchanged. terminated.

-- proof: root is black.

in case 1, parent is black and it's not affected.
in case 2, parent will be the new xnode.
in case 3, parent not changed and becomes case 4.
in case 4, parent is red so it's not root.

all cases exit when xnode is root or is black. and we paint black to 
xnode finally. so root is black.

-- proof: red node's children is black.

in case 1, new red node's children is black and xnode. (xnode still processing).
in case 2, xnode's black and xnode's brother is black. color of brother's children
is black. remove black from them, xnode still black. brother is red. the new xnode 
is black(may be additional black). 
in case 3, it's a rotation, and no rule broken.
in case 4, it's a rotation and xnode removes additional black. xnode's parent is 
black. xnode is black.

*/

    while (node != root_ && color(node) == Black) {
        if (node == node->parent()->left()) {
            BTreeNode *wnode = node->parent()->right();
            if (color(wnode) == Red) {
                set_color(wnode, Black);
                set_color(node->parent(), Red);
                left_rotate(node->parent());
            } else {
                if (color(wnode->left()) == Black && color(wnode->right()) == Black) {
                    set_color(wnode, Red);
                    node = node->parent();
                } else if (color(wnode->right()) == Black) {
                    set_color(wnode->left(), Black);
                    set_color(wnode, Red);
                    right_rotate(wnode);
                } else {
                    set_color(wnode, color(node->parent()));
                    set_color(node->parent(), Black);
                    set_color(wnode->right(), Black);
                    left_rotate(node->parent());
                    node = root_;
                }
            }
        } else {
            BTreeNode *wnode = node->parent()->left();
            if (color(wnode) == Red) {
                set_color(wnode, Black);
                set_color(node->parent(), Black);
                right_rotate(node->parent());
            } else {
                if (color(wnode->left()) == Black && color(wnode->right()) == Black) {
                    set_color(wnode, Red);
                    node = node->parent();
                } else if (color(wnode->left()) == Black) {
                    set_color(wnode->right(), Black);
                    set_color(wnode, Red);
                    left_rotate(wnode);
                } else {
                    set_color(wnode, color(node->parent()));
                    set_color(node->parent(), Black);
                    set_color(wnode->left(), Black);
                    right_rotate(node->parent());
                    node = root_;
                }
            }
        }
    }

    set_color(node, Black);
}

int RBTree::remove(value_t const &value)
{
    BTreeNode *node = locate(value);
    if (node == null_) {
        return -1;
    }
    int ret = remove(node);
    return ret;
}

void RBTree::left_rotate(BTreeNode *node_x)
{
/*
        x
    a          y
            b     c
*/
    do {
        BTreeNode *node_y = node_x->right();

        if (node_y == null_) {
            break;
        }

        BTreeNode *parent = node_x->parent();
        BTreeNode *node_b = node_y->left();
        
        // parent
        if (parent != null_) {
            if (parent->left() == node_x) {
                parent->set_left(node_y);
            } else if (parent->right() == node_x) {
                parent->set_right(node_y);
            } else {
                xlog_err("inner error rotating");
                break;
            }
        } else {
            root_ = node_y;
        }

        // x
        node_x->set_parent(node_y);
        node_x->set_right(node_b);

        // y
        node_y->set_parent(parent);
        node_y->set_left(node_x);

        // b
        if (node_b != null_) {
            node_b->set_parent(node_x);
        }
        
    } while (0);

    return ;
}

void RBTree::right_rotate(BTreeNode *node_y)
{
/*
            y
        x          c
      a   b
*/
    do {
        BTreeNode *node_x = node_y->left();
        if (node_x == null_) {
            break;
        }

        BTreeNode *parent = node_y->parent();
        BTreeNode *node_b = node_x->right();

        // parent
        if (parent != null_) {
            if (parent->left() == node_y) {
                parent->set_left(node_x);
            } else if (parent->right() == node_y) {
                parent->set_right(node_x);
            } else {
                xlog_err("inner error ratating");
                break;
            }
        } else {
            root_ = node_x;
        }

        // x
        node_x->set_parent(parent);
        node_x->set_right(node_y);

        // y
        node_y->set_parent(node_x);
        node_y->set_left(node_b);

        // b
        if (node_b != null_) {
            node_b->set_parent(node_y);
        }
        
    } while (0);

    return ;
}

void RBTree::insert(BTreeNode *node)
{
    BTreeNode *it_p = null_;
    BTreeNode *it = root_;

    while (it != null_) {
        it_p = it;
        if (node->value() < it->value()) {
            it = it->left();
        } else {
            it = it->right();
        }
    }

    // parent
    if (it_p == null_) { // no parent
        root_ = node;
    } else {
        if (node->value() < it_p->value()) {
            it_p->set_left(node);
        } else {
            it_p->set_right(node);
        }
    }

    // node
    node->set_parent(it_p);
    node->set_left(null_);
    node->set_right(null_);
    
    set_color(node, Red);

    insert_fix(node);
    return ;
}

std::string RBTree::dump()
{
    return print_tree(root_, null_);
}

void RBTree::insert_fix(BTreeNode *node)
{

/*

INIT CONDITION:

- z is red node added.
- if z.p is root, then z.p is black.
- rb-tree quality 1(i.e. q1), q3, q5 is ok.

if q2 is not ok, then new node is root.
if q4 is not ok, then new node is child of red node.

only one rule can be broke at init condition.

TERMINATE CONDITION: 

- if z.p is black, then terminate.

KEEP:

- cond1: uncle node is red.
 - make z's parent and uncle black
 - make z's grandparent red
 - only z's grandparent broke q4 or q2;
- cond2: uncle node is black and z is right child.
 - make z to z's parent, left rotate z. into cond3.
- cond3: uncle node is black and z is left child.
 - right rotate z's grandparent.
 - make z's parent black.
 - make z's sibling red.
 - z'parent is black. meet terminate condition.

*/
    while (color(node->parent()) != Black) {
        auto parent = node->parent();
        auto gparent = node->parent()->parent();
        if (parent == gparent->left()) {
            auto uncle = gparent->right();
            if (color(uncle) == Red) {
                set_color(parent, Black);
                set_color(gparent, Red);
                set_color(uncle, Black);
                node = gparent;
            } else {
                if (parent->right() == node) {
                    left_rotate(parent);
                    node = parent;
                } else {
                    set_color(parent, Black);
                    set_color(gparent, Red);
                    right_rotate(gparent);
                }
            }
        } else {
            auto uncle = gparent->left();
            if (color(uncle) == Red) {
                set_color(parent, Black);
                set_color(gparent, Red);
                set_color(uncle, Black);
                node = gparent;
            } else {
                if (parent->left() == node) {
                    right_rotate(parent);
                    node = parent;
                } else {
                    set_color(parent, Black);
                    set_color(gparent, Red);
                    left_rotate(gparent);
                }
            }
        }
    }
    set_color(root_, Black);

}

/* transplant tb to ta */
void RBTree::transplant(BTreeNode *ta, BTreeNode *tb)
{
    BTreeNode *parent = ta->parent();
    if (parent == null_) {
        root_ = tb;
    } else if (parent->left() == ta) {
        parent->set_left(tb);
    } else if (parent->right() == ta) {
        parent->set_right(tb);
    }
    tb->set_parent(parent);
    return ;
}

BTreeNode *RBTree::minimum(BTreeNode *node)
{
    BTreeNode *last = node;
    while (node != null_) {
        last = node;
        if (node->left() != null_) {
            node = node->left();
        } else {
            break;
        }
    }

    return last;
}

BTreeNode *RBTree::locate(value_t const& value)
{
    BTreeNode *node = root_;
    while (node != null_) {
        if (value < node->value()) {
            node = node->left();
        } else if (value > node->value()) {
            node = node->right();
        } else {
            break;
        }
    }
    return node;
}

}