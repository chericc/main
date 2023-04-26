

/**
 * This is a utility file. 
 * 
 * build needs: c++11
 * eg: g++ sort.cpp -std=c++11
 */


#pragma once

#include <iostream>
#include <string>
#include <chrono>

namespace TEST
{

template <typename T>
void print_elements(const T& c, const std::string & optstr = "")
{
    std::cout << optstr << std::endl;
    for (const auto &r : c)
    {
        std::cout << r << " ";
    }
    std::cout << std::endl;
}

}

#define TIME_POINT_BEGIN(name) \
    std::chrono::time_point<std::chrono::system_clock> name; \
    do {    \
        name = std::chrono::system_clock::now();    \
    } while (0)

#define TIME_POINT_END(name) \
    do {    \
        std::chrono::time_point<std::chrono::system_clock> tPointNow =  \
            std::chrono::system_clock::now();   \
        std::chrono::duration<double> duration = tPointNow - name;  \
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() \
            << "ms" << std::endl;   \
    } while (0)
