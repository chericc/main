# adapter

https://springframework.guru/gang-of-four-design-patterns/adapter-pattern/

## introduction

适配器模式把类的接口转换成客户期望的接口。适配器让接口不兼容的类能一起工作（不兼容的类不能修改的情况下）。

适配器可以类比现实生活中的电源插座适配器，电器的插口是固定的，插座的形式很多，因此需要一个适配器来进行形式的转换。

## example

有以下类：  

```C++
class TextFormattable;
class NewLineFormatter;

class CsvFormattable;
class CsvFormatter;

class CsvAdapterImpl;
```

类图如下：

```plantuml
TextFormattable <|-- NewLineFormatter
CsvFormattable <|-- CsvFormatter
TextFormattable <|-- CsvAdapterImpl

abstract class TextFormattable {
    {abstract}+formatText()
    ---
}

class NewLineFormatter {
    +formatText()
    ---
}

abstract class CsvFormattable {
    {abstract}+formatCsvText()
    ---
}

class CsvFormatter {
    +formatCsvText()
    ---
}

class CsvAdapterImpl {
    +formatText()
    ---
    csvFormatter
}

CsvAdapterImpl::csvFormatter --> CsvFormatter
```

其中，客户实际使用的接口类为`TextFormattable`，调用的接口为`formatText`。`CsvFormattable`即为要适配的接口类。

适配器模式的核心为在适配器对象中创建不兼容的类对象，通过“组合”的方式去使用不兼容对象的功能。
