#include "candy_vending_machine.hpp"

#include <assert.h>
#include <stdio.h>

#define PRINT(x...) printf(x)

NoCoinState::NoCoinState(CandyVendingMachine* machine) : p_machine__(machine) {}

void NoCoinState::insertCoin() {
    PRINT("Coin inserted, change to ContainsCoinState\n");
    p_machine__->state__ = p_machine__->contains_coin_state__;
}

void NoCoinState::pressButton() { PRINT("No coin inserted\n"); }

void NoCoinState::dispense() { PRINT("No coin inserted\n"); }

ContainsCoinState::ContainsCoinState(CandyVendingMachine* machine)
    : p_machine__(machine) {}

void ContainsCoinState::insertCoin() { PRINT("Coin already inserted\n"); }

void ContainsCoinState::pressButton() {
    p_machine__->state__ = p_machine__->dispensed_state__;
}

void ContainsCoinState::dispense() { PRINT("Press button to dispense\n"); }

DispensedState::DispensedState(CandyVendingMachine* machine)
    : p_machine__(machine) {}

void DispensedState::insertCoin() {
    PRINT("Error. System is currently dispensing\n");
}

void DispensedState::pressButton() {
    PRINT("Error. System is currently dispensing\n");
}

void DispensedState::dispense() {
    int n_candy_count = p_machine__->count__;

    assert(n_candy_count >= 1);

    --p_machine__->count__;
    if (n_candy_count > 0) {
        p_machine__->state__ = p_machine__->no_coin_state__;
    } else {
        p_machine__->state__ = p_machine__->no_candy_state__;
    }
}

NoCandyState::NoCandyState(CandyVendingMachine* machine)
    : p_machine__(machine) {}

void NoCandyState::insertCoin() { PRINT("No candies avalable\n"); }

void NoCandyState::pressButton() { PRINT("No candies avalable\n"); }

void NoCandyState::dispense() { PRINT("No candies avalable\n"); }

CandyVendingMachine::CandyVendingMachine(int nNumberOfCandies) {
    assert(nNumberOfCandies >= 0);

    count__ = nNumberOfCandies;

    no_coin_state__ = std::make_shared<NoCoinState>(this);
    no_candy_state__ = std::make_shared<NoCandyState>(this);
    dispensed_state__ = std::make_shared<DispensedState>(this);
    contains_coin_state__ = std::make_shared<ContainsCoinState>(this);
    if (nNumberOfCandies > 0) {
        state__ = no_coin_state__;
    } else {
        state__ = no_candy_state__;
    }
}

void CandyVendingMachine::refillCandy(int count) {
    count__ = count;
    if (count__ > 0) {
        state__ = no_coin_state__;
    } else {
        state__ = no_candy_state__;
    }
}

void CandyVendingMachine::ejectCandy() {
    if (count__ > 0) {
        --count__;
    } else {
        PRINT("No candy available\n");
    }

    if (0 == count__) {
        state__ = no_candy_state__;
    }
}

void CandyVendingMachine::insertCoin() {
    PRINT("You inserted a coin.\n");
    state__->insertCoin();
}

void CandyVendingMachine::pressButton() {
    PRINT("You pressed the button.\n");
    state__->pressButton();
    state__->dispense();
}
