#include <stdio.h>

#include <thread>

void hello() { printf("hello world\n"); }

int main() {
    std::thread t(hello);
    t.join();
    return 0;
}