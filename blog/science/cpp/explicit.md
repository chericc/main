# explicit

## 问题

```c++

#include <iostream>

class String
{
public:
    String(int size)
    {
        size_ = size;
    }

    int size_;
};

int main()
{
    String a = 'c';
    String b = 3;
    return 0;
}

```

这里产生了隐式转换，因此可以编译通过。但是语义上不符合逻辑。

在构造函数上增加`explicit`关键字可以避免隐式转换。

## 解决

```c++

#include <iostream>

class String
{
public:
    explicit String(int size)
    {
        size_ = size;
    }

    int size_;
};

int main()
{
    String a = 'c';
    String b = 3;
    return 0;
}

```

编译会报错，如下：

```bash

test.cpp: In function ‘int main()’:
test.cpp:16:16: error: conversion from ‘char’ to non-scalar type ‘String’ requested
   16 |     String a = 'c';
      |                ^~~
test.cpp:17:16: error: conversion from ‘int’ to non-scalar type ‘String’ requested
   17 |     String b = 3;
      |                ^

```