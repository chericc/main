#pragma once

#include <memory>
#include <string>

class Pizza {
   public:
    virtual ~Pizza() = default;
    virtual void addIngredients() = 0;
    void bakePizza();
};

class CheesePizza : public Pizza {
   public:
    void addIngredients() override;
};

class PepperoniPizza : public Pizza {
   public:
    void addIngredients() override;
};

class VeggiePizza : public Pizza {
   public:
    void addIngredients() override;
};

class BasePizzaFactory {
   public:
    virtual ~BasePizzaFactory() = default;
    virtual std::shared_ptr<Pizza> createPizza(std::string type) = 0;
};

class PizzaFactory : public BasePizzaFactory {
   public:
    std::shared_ptr<Pizza> createPizza(std::string type) override;
};