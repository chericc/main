#include <thread>
#include <stdio.h>

void hello()
{
    printf("hello world\n");
}

int main()
{
    std::thread t(hello);
    t.join();
    return 0;
}