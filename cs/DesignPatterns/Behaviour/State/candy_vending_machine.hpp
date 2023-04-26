#pragma once

#include <memory>
#include <string>

class CandyVendingMachine;
class CandyVendingMachineState;
class ContainsCoinState;
class NoCoinState;
class DispensedState;
class NoCandyState;

class CandyVendingMachineState
{
public:
    virtual ~CandyVendingMachineState() = default;
    virtual void insertCoin() = 0;
    virtual void pressButton() = 0;
    virtual void dispense() = 0;
};

class ContainsCoinState : public CandyVendingMachineState
{
public:
    ContainsCoinState (CandyVendingMachine *machine);
    virtual void insertCoin() override;
    virtual void pressButton() override;
    virtual void dispense() override;
private:
    CandyVendingMachine *p_machine__{nullptr};
};

class NoCoinState : public CandyVendingMachineState
{
public:
    NoCoinState(CandyVendingMachine *machine);
    virtual void insertCoin() override;
    virtual void pressButton() override;
    virtual void dispense() override;
private:
    CandyVendingMachine *p_machine__{nullptr};
};

class DispensedState : public CandyVendingMachineState
{
public:
    DispensedState (CandyVendingMachine *machine);
    virtual void insertCoin() override;
    virtual void pressButton() override;
    virtual void dispense() override;
private:
    CandyVendingMachine *p_machine__{nullptr};
};

class NoCandyState : public CandyVendingMachineState
{
public:
    NoCandyState (CandyVendingMachine *machine);
    virtual void insertCoin() override;
    virtual void pressButton() override;
    virtual void dispense() override;
private:
    CandyVendingMachine *p_machine__{nullptr};
};

class CandyVendingMachine
{
public:
    CandyVendingMachine(int nNumberOfCandies);
    void refillCandy(int count);
    void ejectCandy();
    void insertCoin();
    void pressButton();
private:
    std::string toString();
private:
    std::shared_ptr<CandyVendingMachineState> no_coin_state__;
    std::shared_ptr<CandyVendingMachineState> no_candy_state__;
    std::shared_ptr<CandyVendingMachineState> dispensed_state__;
    std::shared_ptr<CandyVendingMachineState> contains_coin_state__;
    std::shared_ptr<CandyVendingMachineState> state__;

    int count__{0};

    friend class NoCoinState;
    friend class ContainsCoinState;
    friend class DispensedState;
    friend class NoCandyState;
};