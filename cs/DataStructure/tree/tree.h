
#pragma once

typedef int ELETYPE;

/**
 * 树节点
 */
class Node {
public:
    Node();
    Node(const ELETYPE &e, int iw = 1);
    ~Node();
    void destroy();
    void replace(Node *ori, Node *rep);
    void leftjoin(Node *node);
    void rightjoin(Node *node);
    Node *left() const;
    Node *right() const;
    int weight() const;

    ELETYPE data;
    Node *l;
    Node *r;
    bool ltag;   /* 为真表示left指向前驱 */
    bool rtag;  /* 为真表示right指向后继 */
    int w;      /* 权 */
};
