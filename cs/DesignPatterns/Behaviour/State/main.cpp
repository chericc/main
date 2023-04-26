#include <iostream>

#include "candy_vending_machine.hpp"

int main()
{
    CandyVendingMachine machine(5);

    while (true)
    {
        int n_choice = 0;

        std::cout << "Choice: [0:insert coin, 1: press button, "
            << "2: eject candy, 3: refill candy]" 
            << std::endl;

        std::cin >> n_choice;

        switch (n_choice)
        {
            case 0:
            {
                machine.insertCoin();
                break;
            }
            case 1:
            {
                machine.pressButton();
                break;
            }
            case 2:
            {
                machine.ejectCandy();
                break;
            }
            case 3:
            {
                machine.refillCandy(5);
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return 0;
}