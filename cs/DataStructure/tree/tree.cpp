#include "tree.h"

#include <iostream>
#include <stack>

Node::Node() {
    data = ELETYPE();
    l = nullptr;
    r = nullptr;
    ltag = false;
    rtag = false;
    w = 1;
}

Node::Node(const ELETYPE& e, int iw) {
    data = e;
    l = nullptr;
    r = nullptr;
    ltag = false;
    rtag = false;
    w = iw;
}

Node::~Node() {
    if (l != nullptr) {
        delete l;
        l = nullptr;
    }
    if (r != nullptr) {
        delete r;
        r = nullptr;
    }
    std::cout << "free " << data << std::endl;
}

void Node::destroy() {
    l = nullptr;
    r = nullptr;
    delete this;
}

void Node::replace(Node* ori, Node* rep) {
    if (l == ori) {
        l = rep;
    } else if (r == ori) {
        r = rep;
    } else {
        throw std::logic_error("");
    }
}

void Node::leftjoin(Node* node) { l = node; }

void Node::rightjoin(Node* node) { r = node; }

Node* Node::left() const { return l; }

Node* Node::right() const { return r; }

int Node::weight() const { return w; }