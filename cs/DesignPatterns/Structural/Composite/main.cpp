#include "product_catalog.h"

int main()
{
    auto p1 = std::make_shared<Product>("Apple", 5);
    auto p2 = std::make_shared<Product>("Orange", 4);

    auto p3 = std::make_shared<Product>("Pen", 1);
    auto p4 = std::make_shared<Product>("Notebook", 0.5);
    auto p5 = std::make_shared<Product>("Ink", 2.5);

    auto c1 = std::make_shared<ProductCatalog>("Fruit");
    auto c2 = std::make_shared<ProductCatalog>("Stationary");
    auto c_main = std::make_shared<ProductCatalog>("Primart catalog");

    c1->add(p1);
    c1->add(p2);

    c2->add(p3);
    c2->add(p4);
    c2->add(p5);

    c_main->add(c1);
    c_main->add(c2);

    c_main->print();

    return 0;
}