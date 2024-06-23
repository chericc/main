#pragma once

/***********

红黑树的性质：

- 每个结点或是红色的，或是黑色的
- 根结点是黑色的
- 叶节点是黑色的
- 红色节点的子节点是黑色的
- 每个节点到其叶节点的所有简单路径上的黑色节点数目相同

隐藏规则：
- 红色节点的父节点是黑色的（如果父节点是红色，则子节点一定是黑色）

*/

#include <list>
#include <memory>
#include <ostream>

typedef int KeyType;
typedef int ValueType;

class RBNode;

typedef std::shared_ptr<RBNode> RBNodePtr;
typedef std::weak_ptr<RBNode> RBNodeWeakPtr;

enum class RBColor { Red, Black };

class RBNode {
   public:
    RBNode(RBNodePtr nil, RBColor color = RBColor::Black);

    RBNodePtr left{};
    RBNodePtr right{};
    RBNodeWeakPtr parent{};

    RBColor color{RBColor::Black};

    KeyType key{};
    ValueType value{};
};

class RBTree {
   public:
    RBTree();
    RBTree(const RBTree& tree);
    ~RBTree();

    void Insert(const KeyType& key, const ValueType& value = ValueType{});
    void Erase(const KeyType& key);
    void Clear();

    RBTree& operator=(const RBTree& tree);

    void GetOrders(std::list<KeyType>& preorder, std::list<KeyType>& midorder,
                   std::list<KeyType>& suborder) const;

   private:
    void LeftRotate(RBNodePtr node);
    void RightRotate(RBNodePtr node);
    void LocateInsertPosition(const KeyType& key, RBNodePtr& it,
                              RBNodePtr& parent) const;
    void InsertFixup(RBNodePtr node);
    RBNodePtr LocateNode(const KeyType& key) const;
    void Transplant(RBNodePtr old_node, RBNodePtr new_node);
    RBNodePtr Min(RBNodePtr node);
    void EraseFixup(RBNodePtr node);

   private:
    RBNodePtr root_;
    RBNodePtr const nil_;

    // friend
    friend std::ostream& operator<<(std::ostream& os, const RBTree& tree);
};
