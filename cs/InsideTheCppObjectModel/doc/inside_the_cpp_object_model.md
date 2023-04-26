# Inside the C++ Object Model

## Chapter 1 - Object Lessons

C++ 在布局以及存取时间上主要的额外负担是由 virtual 引起的，包括：  

- virtual function 机制：用以支持一个有效率的“执行期绑定”  
- virtual base class 机制：用以实现多次出现在继承体系中的 base class ，有一个单一而被共享的实例

### 1.1 The C++ Object Model

在 C++ 中，有两种类成员变量类型：静态成员变量类型和非静态成员变量类型。有三种类成员函数类型：静态成员函数、非静态成员函数和虚函数。  

#### 1.1.1 A Simple Object Model

简单对象模型中，每一个成员都是一个指针，指向实际的数据和函数。  

这个模型并没有应用于实际产品中。  

这个模型可以被作为指向成员的指针的概念中。如以下代码：  

```C++
#include <iostream>
#include <vector>

class Tree
{
public:
    void print_1 (int a)
    {
        std::cout << "print 1" << std::endl;
    }
    void print_2 (int a)
    {
        std::cout << "print 2" << std::endl;
    }
    void print_3 (int a)
    {
        std::cout << "print 3" << std::endl;
    }
};

int main()
{
    typedef void(Tree::*func)(int);

    std::vector<func> vec_func = {& Tree::print_1, & Tree::print_2, & Tree::print_3};

    Tree tree;
    for (auto it : vec_func) 
    {
        (tree.*it)(0);
    }
}
```

这个例子实现了将类成员函数指针存放到数组中，并通过遍历数组的方法用指定的对象依次调用的成员函数调用方式。  

#### 1.1.2 A Table-driven Object Model

对象本身存放两个表格指针，分别指向数据成员表和函数成员表。  

这个模型也不是实际应用于真正的 C++ 编译器上的方案，但是这个方式是虚函数的一个实现方案。  

#### 1.1.3 The C++ Object Model

Stroustrup 设计的 C++ 对象模型中，只有非静态数据成员放在实际的对象中。  

静态数据成员存放在全局类区域，静态函数成员和非静态函数成员也存放在全局类区域。  

对于虚函数，每个对象中存在一个虚函数表指针，指向对应类的虚函数表，而虚函数表中依次存放着所有的虚函数（地址）。  

#### 1.1.4 Adding Inheritance

