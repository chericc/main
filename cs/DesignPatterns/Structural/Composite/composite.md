# Composite

https://springframework.guru/gang-of-four-design-patterns/composite-pattern/

## Introduction

组合模式将对象组合成树状结构以表示部分-整体层次结构。组合模式允许客户统一地处理单个对象和对象的组合。

以一个文件系统的文件夹结构为例，其为树状结构，中间节点和根节点为文件夹，叶节点为文件。

节点的具体类型应该是对客户隐藏的，也即所有节点应具有统一的接口。组合模式就可以用来达到这个目的。

## Example

商店提供一个商品目录，帮助购物者在选购时浏览商品。刚开始的时候，目录中只有少数几个商店生产的商品，随着商店扩张，新的商品陆续加入。这些商品中，有些是从其它制造商购入的。对于不同种类的商品，可以创建子目录，并且这些子目录中随后也会继续创建目录。需求是在一个主目录中高效的管理这些商品和子目录。可以用组合模式来满足这个需求。

- 组合模式将对象组合成树状结构以表示部分-整体层次结构：部分-整体层次结构由小的独立的“部分”对象和大的“整体”对象组成，其中整体对象是聚合对象（Aggregation）。
- 组合模式允许客户统一地处理单个对象和对象的组合：客户可以对聚合对象（整体对象）和独立对象（部分对象）进行相同的操作。

商品目录的实现：

- 目录（组件，Component）：树结构中对象的抽象基类。定义了所有对象的行为以及访问和管理子组件的方式。
- 产品（叶节点，Leaf）：组件的子类，没有子节点。
- 产品目录（组合，Composite）：组件的子类，表示树结构中有子节点的节点。可以存储叶节点，并且定义了组件中给出的对子组件的访问和管理行为。子节点可以是一个或多个叶节点，也可以是其它组件。
- 客户：和组件交互，访问并操纵实例中的对象。

**聚合**：

产品类和产品目录类均继承自产品组件类，也即为继承关系。产品目录可以包含作为产品组件的产品以及其它产品目录对象，这种关系就称为聚合。

## Classes

```plantuml
CatalogComponent <|-- Product: Inheritance
CatalogComponent <|-- ProductCatalog: Inheritance

ProductCatalog::catalogComponentList o-- CatalogComponent: Aggregation

Client -right-> CatalogComponent

abstract class CatalogComponent {
    add(catalogComponent)
    remove(catalogComponent)
    getName()
    getPrice()
    ---
}

class Product {
    getName()
    getPrice()
    print()
    ---
    .name
    .price
}

class ProductCatalog {
    getName()
    print()
    add()
    remove()
    ---
    .name
    .catalogComponentList
}

note "Leaf" as N1
note "Composite" as N2

Product .. N1
ProductCatalog .. N2

```

