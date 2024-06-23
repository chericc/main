
#include "product_catalog.hpp"

#include <iostream>

void CatalogComponent::add(std::shared_ptr<CatalogComponent> cc) {
    std::cout << "call not support" << std::endl;
    std::terminate();
}

void CatalogComponent::remove(std::shared_ptr<CatalogComponent> cc) {
    std::cout << "call not support" << std::endl;
    std::terminate();
}

std::string CatalogComponent::getName() {
    std::cout << "call not support" << std::endl;
    std::terminate();
}

double CatalogComponent::getPrice() {
    std::cout << "call not support" << std::endl;
    std::terminate();
}

void CatalogComponent::print() {
    std::cout << "call not support" << std::endl;
    std::terminate();
}

Product::Product(std::string name, double price) {
    name_ = name;
    price_ = price;
}

std::string Product::getName() { return name_; }

double Product::getPrice() { return price_; }

void Product::print() {
    std::cout << "Product name:" << name_ << " Price:" << price_ << std::endl;
}

ProductCatalog::ProductCatalog(std::string name) { name_ = name; }

std::string ProductCatalog::getName() { return name_; }

void ProductCatalog::print() {
    for (auto item : items_) {
        item->print();
    }
}

void ProductCatalog::add(std::shared_ptr<CatalogComponent> cc) {
    items_.push_back(cc);
}

void ProductCatalog::remove(std::shared_ptr<CatalogComponent> cc) {
    for (auto it = items_.cbegin(); it != items_.cend(); ++it) {
        if (*it == cc) {
            items_.erase(it);
            break;
        }
    }
}