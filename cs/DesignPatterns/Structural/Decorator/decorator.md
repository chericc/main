# Decorator

https://springframework.guru/gang-of-four-design-patterns/decorator-pattern/

## Introduction

可以通过继承的方式扩展一个已有功能，但是这种方式是在编译时静态确定的，因此往往不够灵活。当需要对功能进行修改时，必须要对代码进行修改，这违背了开闭原则。

**装饰模式**的目的就是动态的为对象添加新的功能。使用了修饰模式后，在添加新功能时，对象不需要关注新的功能，这一点符合开闭原则。

## Example

考虑一个卖花的场景，有玫瑰花、兰花等等。则构建类层次如下：

- FlowerBouquet：花束
- Rose：玫瑰 
- Orchid：兰花

```plantuml
FlowerBouquet <|-- RoseBouquet
FlowerBouquet <|-- OrchidBouquet
```

客户买花的时候，可能有纸包装、丝带蝴蝶结、装饰闪光片等等要求并支付额外的费用。

为了实现这个需求，如果简单的用继承来扩展，由于需求的类型很多并且存在很多组合，因此子类会很多，出现了类爆炸。

- PaperWrapped：纸包装
- RibbonBow：绑蝴蝶结
- Glitter：装饰闪光片

```plantuml
FlowerBouquet <|-- RoseBouquet
FlowerBouquet <|-- RoseBouquetPaperWrapped
FlowerBouquet <|-- RoseBouquetRibbonBow
FlowerBouquet <|-- Others
```

这是一个使用装饰模式的理想场景。使用装饰模式之后的类设计如下：

```plantuml
FlowerBouquet <|-- RoseBouquet
FlowerBouquet <|-- OrchidBouquet
FlowerBouquet <|-- FlowerBouquetDecorator
FlowerBouquetDecorator <|-- PaperWrapper
FlowerBouquetDecorator <|-- RibbonBow
FlowerBouquetDecorator <|-- Glitter

FlowerBouquetDecorator o-- FlowerBouquet 

class FlowerBouquet {
    getDescription()
    cost()
    ---
    description
}

```

注意，其中`FlowerBouquetDecorator`和`FlowerBouquet`有两种关系：

1. 继承关系。装饰类本身也是一个组件。
2. 组合关系。装饰类包含一个组件对象，利用这个组件对象增加特性。

