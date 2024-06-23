/**
 * compile commands:
 * g++ -I.. -std=c++11 prime_number.cpp
 */

#include <iostream>
#include <vector>

bool IsPrimeNumber(std::size_t number) {
    for (std::size_t i = 2; i < number / 2 + 1; ++i) {
        if (number % i == 0) {
            return false;
        }
    }

    return true;
}

int main() {
    std::size_t nSize = 1000 * 1000;

    std::vector<std::size_t> vecData(nSize);

    for (std::size_t i = 0; i < nSize; ++i) {
        vecData[i] = i + 1;
    }

    for (std::size_t i = 0; i < nSize; ++i) {
        if (IsPrimeNumber(vecData[i])) {
            std::cout << vecData[i] << std::endl;
        }
    }

    return 0;
}
