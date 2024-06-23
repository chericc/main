#include "pizza.hpp"

int main() {
    auto pizzaFactory = std::make_shared<PizzaFactory>();

    auto cheesePizza = pizzaFactory->createPizza("cheese");
    auto veggiePizza = pizzaFactory->createPizza("veggie");

    return 0;
}