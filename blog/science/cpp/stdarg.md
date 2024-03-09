# stdarg

## Question

- How does va_arg (and so on) implemented?
- What will happen if you call printf("%d", char)?

## Answer

- Question 1: 
- It is not implemented as a user program(not even in libc). It acts as if the C keyword `sizeof`. At compiling time, this word will be parsed by the compiler and do something to implement its funciton.
- Question 2:
- `printf` uses va_arg to get all the arguments. If you write `%d`, then it will assume that the argument is an integer(of type `int`). This may result in error because content is referenced by pointers.