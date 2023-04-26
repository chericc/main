#include <functional>
#include <thread>
#include <iostream>

class Tree
{
public:
    void start()
    {
        std::function<void()> func([this]{this->doStart();});
        std::thread trd(func);
        trd.join();
    }
private:
    void doStart()
    {
        for (int i = 0; i < 5; ++i)
        {
            std::cout << i << std::endl;
        }
        return ;
    }
};

int main()
{
    Tree tree;
    tree.start();
}