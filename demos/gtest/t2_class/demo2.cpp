#include "demo2.hpp"

int Calculator::setValue(int a, int b) {
    value_a = a;
    value_b = b;
    return 0;
}

int Calculator::add() { return value_a + value_b; }

int Calculator::minus() { return value_a - value_b; }

int Calculator::multiply() { return value_a * value_b; }

int Calculator::divide() { return value_a / value_b; }