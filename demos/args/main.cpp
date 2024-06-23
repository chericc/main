#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <array>
#include <iostream>

void print_usage(int argc, char** argv) {
    std::cout << "Usage: %s -a a -b b -c c" << std::endl;
    return;
}

void test_arg(int argc, char* argv[]) {
    int opt = 0;

    argv -= 1;

    while ((opt = getopt(argc, argv, "a:b:c:")) != -1) {
        switch (opt) {
            case 'a': {
                std::cout << "option for a is:" << optarg << std::endl;
                break;
            }
            case 'b': {
                std::cout << "option for b is:" << optarg << std::endl;
                break;
            }
            case 'c': {
                std::cout << "option for c is:" << optarg << std::endl;
                break;
            }
            default: {
                print_usage(argc, argv);
                exit(0);
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    test_arg(argc, &argv[1]);
    return 0;
}