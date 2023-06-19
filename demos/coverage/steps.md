# steps of coverage with gcov

## about

主要用来统计被测代码的覆盖率；

也可以用来统计代码执行次数（辅助帮助性能分析）；

## src

```c++
// test.cpp
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
```

## eg1

```bash
g++ -c --coverage test.cpp -o test.o
g++ test.o --coverage -o test.out

./test.out

# gcov
gcov -m test.cpp

cat test.cpp.gcov
        -:    0:Source:../test.cpp
        -:    0:Graph:test.gcno
        -:    0:Data:test.gcda
        -:    0:Runs:1
        -:    1:#include <stdio.h>
        -:    2:
        1:    3:int add(int a, int b)
        -:    4:{
        1:    5:    return a + b;
        -:    6:}
        -:    7:
        1:    8:int minus(int a, int b)
        -:    9:{
        1:   10:    return a - b;
        -:   11:}
        -:   12:
        1:   13:int main()
        -:   14:{
        1:   15:    int c = add(1,2);
        1:   16:    int d = minus(3,4);
        1:   17:    return c;
        -:   18:}

# lcov
lcov -c -d . -o test.info
genhtml -o test_coverage test.info
```

