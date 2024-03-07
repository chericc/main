# 右值和右值引用

## ref

https://en.cppreference.com/w/cpp/language/reference

https://www.cnblogs.com/KillerAery/p/12802771.html

## 左值和右值

- 左值：表达式结束之后仍然存在的持久对象；
- 右值：表达式结束后就不存在的临时对象；

## 右值引用的来源

### 临时对象作为参数传值的一些问题

考虑形式如 `push(Object())` 的写法。这里 `push` 不能写成 `push(Object &)` ，因此函数中如果要修改这个对象，则必然存在拷贝。如果写成 `push(Object &&)` ，则临时对象就会被转移到函数内，就不需要拷贝了。

### 右值引用和左值引用构成了重载关系



## std::move

## std::ref