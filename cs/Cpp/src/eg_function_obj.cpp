#include <functional>
#include <iostream>

class Tree
{
public:
    void print ()
    {
        print__();
    }
private:
    void print__()
    {
        std::cout << "This is a tree" << std::endl;
    }
};

typedef void (Tree::*Func)();

int main()
{
    {
        Tree tree;
        
        std::function<void(void)> func = std::bind (& Tree::print, & tree);
        func ();
    }

    {
        Tree tree;
        Func func = &Tree::print;

        /* Note here: '.*' is a pointer-to-member-function operator of C++  */

        // (tree.*func)();
        // std::invoke (func, tree);
    }

    return 0;
}