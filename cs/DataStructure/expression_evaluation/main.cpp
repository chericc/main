
#include <iostream>
#include <list>
#include <string>

#include "exprcalc.h"

int main() {
    ExprCalc calc;

    while (true) {
        std::string s;
        std::cin >> s;
        auto list = ExprItem::parse(s);
        for (auto item : list) {
            std::cout << item;
        }
        std::cout << std::endl;

        double d = calc.calc (list);
        std::cout << "result=" << d << std::endl;

        std::cout << "finish" << std::endl;
    }
    
    return 0;
}
