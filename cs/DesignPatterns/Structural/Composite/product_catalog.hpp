#pragma once

#include <exception>
#include <list>
#include <memory>
#include <string>

/**
 * 注意：这里接口需要实现一部分，在实现中禁止调用
 * 因为子类中不一定有所有的实现。
 */
class CatalogComponent {
   public:
    virtual void add(std::shared_ptr<CatalogComponent> cc);
    virtual void remove(std::shared_ptr<CatalogComponent> cc);
    virtual std::string getName();
    virtual double getPrice();
    virtual void print();
};

class Product : public CatalogComponent {
   public:
    Product(std::string name, double price);
    std::string getName() override;
    double getPrice() override;
    void print() override;

   protected:
    std::string name_;
    double price_;
};

class ProductCatalog : public CatalogComponent {
   public:
    ProductCatalog(std::string name);
    std::string getName() override;
    void print() override;
    void add(std::shared_ptr<CatalogComponent> cc) override;
    void remove(std::shared_ptr<CatalogComponent> cc) override;

   protected:
    std::list<std::shared_ptr<CatalogComponent> > items_;
    std::string name_;
};