# prototype

参考来源：  

> <https://springframework.guru/gang-of-four-design-patterns/prototype-pattern/>

## What is Prototype Pattern

Prototype 原型

在创建一个对象的时候，不是创建一个新对象，而是在最开始创建一个原型对象，后续创建对象时通过拷贝这个对象得到。

原型解决的问题是，某些场景下，创建一个对象可能非常耗时（也即代价很大），直接拷贝一个对象会节省时间。

## Example

```c++
abstract class PrototypeCapableDocument {
    +getVendorName()
    +setVendorName()
    +getContent()
    +setContent()
    {abstract}+cloneDocument()
    ---
    vendorName
    content
}
```