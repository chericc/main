#include <stdio.h>

#include <thread>

int main() {
    unsigned int i = std::thread::hardware_concurrency();

    printf("c=%u\n", i);
    return 0;
}