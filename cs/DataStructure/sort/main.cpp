
#include <algorithm>
#include <functional>
#include <iostream>

#include "sort.h"

void print(const std::vector<int>& ref) {
    for (auto item : ref) {
        std::cout << item << " ";
    }
}

int main() {
    std::vector<std::function<void(std::vector<int>&)> > fs;
    fs.push_back(sort_insert);
    fs.push_back(sort_insert_half);
    fs.push_back(sort_shell);
    fs.push_back(sort_bubble);
    fs.push_back(sort_fast);
    fs.push_back(sort_heap);

    for (int m = 0; m < 100; ++m) {
        // std::vector<int> ori = {0,1,2,3,4,5,6,7,8,9};
        std::vector<int> ori = {0, 1, 2, 3, 4, 5, 6};
        std::vector<int> dst = ori;
        std::random_shuffle(dst.begin(), dst.end());

        int berror = 0;

        for (auto it = fs.cbegin(); it != fs.cend(); ++it) {
            std::vector<int> temp = dst;
            (*it)(temp);
            if (temp != ori) {
                std::cout << std::endl;
                std::cout << "shuf:";
                print(dst);
                std::cout << std::endl;
                std::cout << "temp:";
                print(temp);
                std::cout << std::endl;
                std::cout << "ori:";
                print(ori);
                std::cout << std::endl;

                std::cout << "error!" << std::endl;
                berror = 1;
                break;
            }
        }

        if (berror) {
            break;
        }
    }

    return 0;
}