#include "pizza.hpp"

#include "xlog.h"

void Pizza::bakePizza() { xlog_dbg("Pizza baked at 400 for 20 minutes"); }

void CheesePizza::addIngredients() {
    xlog_dbg("Preparing ingredients for cheese pizza");
}

void PepperoniPizza::addIngredients() {
    xlog_dbg("Preparing ingredients for pepperoni pizza");
}

void VeggiePizza::addIngredients() {
    xlog_dbg("Preparing ingredients for veggie pizza");
}

std::shared_ptr<Pizza> PizzaFactory::createPizza(std::string type) {
    std::shared_ptr<Pizza> pizza;

    if (type == "cheese") {
        pizza = std::make_shared<CheesePizza>();
    } else if (type == "pepperoni") {
        pizza = std::make_shared<PepperoniPizza>();
    } else if (type == "veggie") {
        pizza = std::make_shared<VeggiePizza>();
    } else {
        xlog_err("Type not support");
    }

    pizza->addIngredients();
    pizza->bakePizza();

    return pizza;
}