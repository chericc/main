#include <stdio.h>

int add(int a, int b)
{
    return a + b;
}

int minus(int a, int b)
{
    return a - b;
}

int main()
{
    int c = add(1,2);
    int d = minus(3,4);
    return c;
}