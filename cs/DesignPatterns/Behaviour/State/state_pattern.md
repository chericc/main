# state pattern

@ref
<https://springframework.guru/gang-of-four-design-patterns/state-pattern/>

## example

```plantuml
title CandyVendingMachine
state a <<choice>>
[*] -down-> NoCoinState
NoCoinState -right-> ContainsCoinState: insert coin
ContainsCoinState --> DispensedState: press button
DispensedState --> a: dispense
a --> NoCoinState: [candy > 0]
a --> NoCandyState: [candy = 0]
```

上述状态的变化仅仅描述了状态的自变化（状态接口引起的），不包括外部变化。  
上述状态的其他状态接口都不会引起状态变化。  

## class diagram

```plantuml

class Context {
    +Request()
    ---
    -state
}
note left of Context::Request
    state->Handle()
end note

Context::state -right-> State

State <|-- ConcreteStateA
State <|-- ConcreteStateB

class State {
    +Handle()
    ---
}

class ConcreteStateA {
    +Handle()
    ---
}

class ConcreteStateB{
    +Handle
    ---
}

```

说明：

外部功能通过 Context 提供。不同的状态下的不同功能通过 state 的不同体现。