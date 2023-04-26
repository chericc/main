#pragma once

class Calculator
{
public:
    int setValue(int a, int b);

    int add();
    int minus();
    int multiply();
    int divide();
private:
    int value_a{0};
    int value_b{0};
};