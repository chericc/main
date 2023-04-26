# understand function object

## 观察以下这段代码

```C++
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

int main()
{
    Tree tree;
    
    std::function<void(void)> func = std::bind (& Tree::print, & tree);
    func ();

    return 0;
}
```

