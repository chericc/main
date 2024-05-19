#include "rbtree.hpp"

#include <assert.h>
#include <functional>

#include "log.hpp"

RBNode::RBNode(RBNodePtr nil, RBColor color_)
    :left(nil),right(nil),parent(nil),color(color_)
{
}

RBTree::RBTree()
    : nil_(std::make_shared<RBNode>(nullptr))
{
    root_ = nil_;
}

RBTree::RBTree(const RBTree &tree)
    : nil_(std::make_shared<RBNode>(nullptr))
{
    *this = tree;
}

RBTree::~RBTree()
{
    // nothing
}

void RBTree::Insert(const KeyType &key, const ValueType &value)
{
/*

     y(R)
     x(B)
-->
     y(R)
     z(R)
     x(B)
     


*/

    RBNodePtr y;
    RBNodePtr x;
    RBNodePtr z = std::make_shared<RBNode>(nil_);
    z->color = RBColor::Red;
    z->key = key;
    z->value = value;

    LocateInsertPosition(key, x, y);

    z->parent = y;
    if (nil_ == y)
    {
        root_ = z;
    }
    else if (z->key < y->key)
    {
        y->left = z;
    }
    else 
    {
        y->right = z;
    }

    InsertFixup(z);
}

void RBTree::Erase(const KeyType &key)
{
    RBNodePtr node = LocateNode(key);
    if (node == nil_)
    {
        return ;
    }

    RBNodePtr x;
    RBNodePtr y = node;
    RBNodePtr z = node;
    RBColor y_original_color = y->color;

    if (z->left == nil_)
    {
        /**
         *          z
         *    nil         x
         *              -   -
         * ------------>
         *       x
         *     -   -
         */
        x = z->right;
        Transplant(z, z->right);
    }
    else if (z->right == nil_)
    {
        /**
         *       z
         *   x     nil
         * -   -
         * --------------->
         *       x
         *     -   -
         */
        x = z->left;
        Transplant(z, z->left);
    }
    else
    {
        /**
         *        z
         *              ...
         *              y
         *                 x
         */
        y = Min(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent.lock() == z)
        {
            /**
             *     z
             *        y
             *          x
             */
            x->parent = y;   // TODO: redundant operation
        }
        else
        {
            /**
             *        z
             *   -            -
             *            y        -
             *              x
             * -------------------->
             *         z
             *   -            -
             *              x    -
             */
            Transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        Transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == RBColor::Black)
    {
        EraseFixup(x);
    }

    return ;
}

void RBTree::Clear()
{
    root_ = nil_;
}

RBTree &RBTree::operator=(const RBTree &tree)
{
    std::function<RBNodePtr(const RBNodePtr src, RBNodePtr parent)> func_make_copy;
    func_make_copy = [&](const RBNodePtr src, RBNodePtr parent)->RBNodePtr
    {
        if (src == tree.nil_)
        {
            return nil_;
        }

        RBNodePtr node = std::make_shared<RBNode>(nil_);
        node->color = src->color;
        node->key = src->key;
        node->parent = parent;
        node->value = src->value;
        if (src->left != tree.nil_)
        {
            node->left = func_make_copy(src->left, node);
        }
        else
        {
            node->left = nil_;
        }
        if (src->right != tree.nil_)
        {
            node->right = func_make_copy(src->right, node);
        }
        else
        {
            node->right = nil_;
        }
        return node;
    };

    root_ = func_make_copy(tree.root_, nil_);

    return *this;
}

void RBTree::LeftRotate(RBNodePtr node)
{
    /**

            p
            x
        a           y
                b   c

    --->
            p
            y
        x             c
    a   b

    */

    assert(node != nil_);
    assert(node->right != nil_);

    RBNodePtr p = node->parent.lock();
    RBNodePtr x = node;
    RBNodePtr y = node->right;
    RBNodePtr a = node->left;
    RBNodePtr b = y->left;
    RBNodePtr c = y->right;

    if (p == nil_)
    {
        root_ = y;
    }
    else if (p->left == x)
    {
        p->left = y;
    }
    else
    {
        p->right = y;
    }

    x->parent = y;
    x->right = b;

    y->parent = p;
    y->left = x;

    if (b != nil_)
    {
        b->parent = x;
    }
}

void RBTree::RightRotate(RBNodePtr node)
{
    /**

            p
            y
        x             c
    a   b
    --->
            p
            x
        a           y
                b   c


    */

    assert(node != nil_);
    assert(node->left != nil_);

    RBNodePtr p = node->parent.lock();
    RBNodePtr y = node;
    RBNodePtr x = node->left;
    RBNodePtr a = x->left;
    RBNodePtr b = x->right;
    RBNodePtr c = y->right;

    if (p == nil_)
    {
        root_ = x;
    }
    else if (p->left == y)
    {
        p->left = x;
    }
    else 
    {
        p->right = x;
    }

    y->parent = x;
    y->left = b;

    x->parent = p;
    x->right = y;

    if (b != nil_)
    {
        b->parent = y;
    }
}

void RBTree::LocateInsertPosition(const KeyType &key, RBNodePtr &it, RBNodePtr &parent) const
{
    RBNodePtr y = nil_;
    RBNodePtr x = root_;

    while (x != nil_)
    {
        y = x;
        if (key < x->key)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }
    it = x;
    parent = y;

    return ;
}

void RBTree::InsertFixup(RBNodePtr z)
{
    while (RBColor::Red == z->parent.lock()->color)
    {
        if (z->parent.lock() == z->parent.lock()->parent.lock()->left)
        {
            RBNodePtr y = z->parent.lock()->parent.lock()->right;
            if (y->color == RBColor::Red)
            {
                /**
                 * condition 1-1
                 *         B
                 *     R       R(y)
                 * R(z)
                 * ------------------>
                 *         R
                 *     B       B(y)
                 * R(z)
                 * ------------------>
                 *          R(z)
                 *     B          B(y)
                 *  R
                 */
                z->parent.lock()->color = RBColor::Black;
                z->parent.lock()->parent.lock()->color = RBColor::Red;
                y->color = RBColor::Black;
                z = z->parent.lock()->parent.lock();
            }
            else 
            {
                if (z == z->parent.lock()->right)
                {
                    /**
                     * condition 1-2
                     *           B
                     *     R            B(y)
                     *       R(z)
                     * ----------------------->
                     *             B
                     *     R(z)          B(y)
                     *          R
                     * ----------------------->
                     *           B
                     *     R            B(y)
                     * R(z)
                     */
                    z = z->parent.lock();
                    LeftRotate(z);
                }
                else
                {
                    /**
                     * condition 1-3
                     *           B
                     *    R              B(y)
                     * R(z)
                     * ------------------------>
                     *           R
                     *    B              B(y)
                     * R(z)
                     * ------------------------>
                     *           B
                     *      R(z)       R
                     *                      B(y)
                     */
                    z->parent.lock()->color = RBColor::Black;
                    z->parent.lock()->parent.lock()->color = RBColor::Red;
                    RightRotate(z->parent.lock()->parent.lock());
                }
            }
        }
        else
        {
            RBNodePtr y = z->parent.lock()->parent.lock()->left;
            if (y->color == RBColor::Red)
            {
                /**
                 * condition 2-1
                 *              B
                 *         R(y)      R
                 *                       R(z)
                 * ---------------------------->
                 *              R
                 *         B(y)      B
                 *                       R(z)
                 * ---------------------------->
                 *              R(z)
                 *         B(y)       B
                 *                        R
                 */
                z->parent.lock()->color = RBColor::Black;
                z->parent.lock()->parent.lock()->color = RBColor::Red;
                y->color = RBColor::Black;
                z = z->parent.lock()->parent.lock();
            }
            else
            {
                if (z == z->parent.lock()->left)
                {
                    /**
                     * condition 2-2
                     *              B
                     *         B(y)          R
                     *                   R(z)
                     * --------------------------->
                     *              B
                     *        B(y)         R(z)
                     *                   R
                     * --------------------------->
                     *              B
                     *         B(y)       R
                     *                      R(z)
                     */
                    z = z->parent.lock();
                    RightRotate(z);
                }
                else
                {
                    /**
                     * condition 2-3
                     *              B
                     *         B(y)      R
                     *                      R(z)
                     * ---------------------------->
                     *              R
                     *         B(y)      B
                     *                      R(z)
                     * ---------------------------->
                     *              B
                     *         R         R(z)
                     *     B(y)             
                     */
                    z->parent.lock()->color = RBColor::Black;
                    z->parent.lock()->parent.lock()->color = RBColor::Red;
                    LeftRotate(z->parent.lock()->parent.lock());
                }
            }
        }
    }
    root_->color = RBColor::Black;
}

RBNodePtr RBTree::LocateNode(const KeyType &key) const
{
    RBNodePtr node = root_;
    while (node != nil_)
    {
        if (key == node->key)
        {
            break;
        }
        else if (key < node->key)
        {
            node = node->left;
        }
        else 
        {
            node = node->right;
        }
    }
    return node;
}

std::ostream &operator<<(std::ostream &os, const RBTree &tree)
{
    std::list<KeyType> preorder, midorder, suborder;

    tree.GetOrders(preorder, midorder, suborder);

    os << "pre order:" << std::endl;
    for (auto &it : preorder)
    {
        os << it << " ";
    }
    os << std::endl;
    
    os << "mid order:" << std::endl;
    for (auto &it : midorder)
    {
        os << it << " ";
    }
    os << std::endl;

    os << "sub order:" << std::endl;
    for (auto &it : suborder)
    {
        os << it << " ";
    }
    os << std::endl;

    return os;
}

void RBTree::GetOrders(std::list<KeyType> &preorder, std::list<KeyType> &midorder, std::list<KeyType> &suborder) const
{
    std::function<void(RBNodePtr,RBNodePtr)> func_pre_order_print;
    std::function<void(RBNodePtr,RBNodePtr)> func_mid_order_print;
    std::function<void(RBNodePtr,RBNodePtr)> func_sub_order_print;
    func_pre_order_print = [&](RBNodePtr node, RBNodePtr nil)
    {
        if (node != nil)
        {
            preorder.push_back(node->key);
            if (node->left != nil)
            {
                func_pre_order_print(node->left, nil);
            }
            if (node->right != nil)
            {
                func_pre_order_print(node->right, nil);
            }
        }
    };
    func_mid_order_print = [&](RBNodePtr node, RBNodePtr nil)
    {
        if (node != nil)
        {
            if (node->left != nil)
            {
                func_mid_order_print(node->left, nil);
            }
            midorder.push_back(node->key);
            if (node->right != nil)
            {
                func_mid_order_print(node->right, nil);
            }
        }
    };
    func_sub_order_print = [&](RBNodePtr node, RBNodePtr nil)
    {
        if (node != nil)
        {
            if (node->left != nil)
            {
                func_sub_order_print(node->left, nil);
            }
            if (node->right != nil)
            {
                func_sub_order_print(node->right, nil);
            }
            suborder.push_back(node->key);
        }
    };

    preorder.clear();
    midorder.clear();
    suborder.clear();
    func_pre_order_print(root_, nil_);
    func_mid_order_print(root_, nil_);
    func_sub_order_print(root_, nil_);

    return ;
}

/**
 * 将 old_node 节点用 new_node 表示的子树替换。
*/
void RBTree::Transplant(RBNodePtr old_node, RBNodePtr new_node)
{
    if (old_node->parent.lock() == nil_)
    {
        root_ = new_node;
    }
    else if (old_node == old_node->parent.lock()->left)
    {
        old_node->parent.lock()->left = new_node;
    }
    else
    {
        old_node->parent.lock()->right = new_node;
    }
    new_node->parent = old_node->parent.lock();

    return ;
}

RBNodePtr RBTree::Min(RBNodePtr node)
{
    RBNodePtr z = node;
    while (z != nil_)
    {
        if (z->left != nil_)
        {
            z = z->left;
        }
        break;
    }
    return z;
}

void RBTree::EraseFixup(RBNodePtr node)
{
    RBNodePtr x = node;

    while (x != root_ && x->color == RBColor::Black)
    {
        if (x == x->parent.lock()->left)
        {
            RBNodePtr w = x->parent.lock()->right;
            if (w->color == RBColor::Red)
            {
                /**
                 * case erasefixup::1-1
                 *              B
                 *        B(x)       R(w)
                 *                 B
                 * ----------------------->
                 *              R
                 *        B(x)       B(w)
                 *                 B
                 * ----------------------->
                 *              B(w)
                 *        R
                 *    B(x)  B
                 * ----------------------->
                 *              B
                 *        R
                 *    B(x)  B(w)
                 */
                w->color = RBColor::Black;
                x->parent.lock()->color = RBColor::Red;
                LeftRotate(x->parent.lock());
                w = x->parent.lock()->right;
            }
            else if (w->left->color == RBColor::Black && w->right->color == RBColor::Black)
            {
                /**
                 * case erasefixup::1-2
                 *               *
                 *       B(x)          B(w)
                 *                   B    B
                 * --------------------------->
                 *             *(x)
                 *       B             R(w)
                 *                    B   B
                 */
                w->color = RBColor::Red;
                x = x->parent.lock();
            }
            else if (w->right->color == RBColor::Black)
            {
                /**
                 * case erasefixup::1-3
                 *                *
                 *       R(x)               B(w)
                 *                       R      B
                 * ------------------------------>
                 *                *
                 *       B(x)               R(w)
                 *                       B       B
                 * ------------------------------>
                 *                *
                 *       B(x)               B(w)
                 *                             R
                 *                                B
                 */
                w->left->color = RBColor::Black;
                w->color= RBColor::Red;
                RightRotate(w);
                w = x->parent.lock()->right;
            }
            else
            {
                /**
                 * case erasefixup::1-4
                 *               a*
                 *      B(x)                B(w)
                 *                       b*       R
                 * --------------------------------->
                 *               B
                 *      B(x)                a*(w)
                 *                       b*      B
                 * --------------------------------->
                 *               a*(w)
                 *      B                     B
                 *  B(x)   b*
                 * --------------------------------->
                 *               a*(w)
                 *      B                     B
                 *  B     b*
                 *          
                 */
                w->color = x->parent.lock()->color;
                x->parent.lock()->color = RBColor::Black;
                w->right->color = RBColor::Black;
                LeftRotate(x->parent.lock());
                x = root_;
            }
        }
        else 
        {
            RBNodePtr w = x->parent.lock()->left;
            if (w->color == RBColor::Red)
            {
                /**
                 * case erasefixup::2-1
                 *               B
                 *      R(w)            B(x)
                 *          B
                 * ------------------------->
                 *               R
                 *      B(w)            B(x)
                 *          B
                 * ------------------------->
                 *               B(w)
                 *                      R
                 *                   B     B(x)
                 * ------------------------->
                 *               B
                 *                        R
                 *                   B(w)     B(x)
                 */
                w->color = RBColor::Black;
                x->parent.lock()->color = RBColor::Red;
                RightRotate(x->parent.lock());
                w = x->parent.lock()->left;
            }
            else if (w->left->color == RBColor::Black && w->right->color == RBColor::Black)
            {
                /**
                 * case erasefixup::2-2
                 *             *
                 *      B(w)         B(x)
                 *    B     B
                 * ----------------------->
                 *             *(x)
                 *      R(w)
                 *    B     B        B
                 */
                w->color = RBColor::Red;
                x = x->parent.lock();
            }
            else if (w->left->color == RBColor::Black)
            {
                /**
                 * case eerasefixup::2-3
                 *                 *
                 *        B(w)              R(x)
                 *      B     R
                 * ------------------------------>
                 *                 *
                 *        R(w)              R(x)
                 *      B     B
                 * ------------------------------>
                 *                 *
                 *          B(w)          R(x)
                 *       R
                 *     B
                 */
                w->right->color = RBColor::Black;
                w->color = RBColor::Red;
                LeftRotate(w);
                w = x->parent.lock()->left;
            }
            else
            {
                /**
                 * case eerasefixup::2-4
                 *                a*
                 *        B(w)              B(x)
                 *      R      b*
                 * ----------------------------->
                 *                B
                 *        a*(w)             B(x)
                 *      B     b*
                 * ----------------------------->
                 *                a*(w)
                 *          B                B
                 *                        b*   B(x)
                 */
                w->color = x->parent.lock()->color;
                x->parent.lock()->color = RBColor::Black;
                w->left->color = RBColor::Black;
                RightRotate(x->parent.lock());
                x = root_;
            }
        }
    }
    x->color = RBColor::Black;
}