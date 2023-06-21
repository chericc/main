# sanitizer

## docs

https://github.com/google/sanitizers

## classes

AddressSanitizer: detects addressability issues;

LeakSanitizer: detects memory leaks;

ThreadSanitizer: detects data races and dead locks;

MemorySanitizer: detects use of uninitialized memory;

## AddressSanitizer

### asan

Abbreviation: asan

### detects

- Use after free
- Heap buffer overflow
- Stack buffer overflow
- Global buffer overflow
- Use after return
- Use after scope
- Initialization order bugs
- Memory leaks

### performance

~2x;

### technique

#### general

The run-time library replaces the `malloc` and `free` functions. 

Some memory is poisoned and every memory access is transformed in the following way:

```c
*address = ... 
... = *address

--> 

if (is_poisoned(address))
{
    report error;
}
*address = ... 
... = *address
```

#### memory mapping and instrumentation

- Main application memory
- Shadow memory

Poisoning a byte in the main memory means writing some special value into the corresponding shadow memory.

#### run-time library(replaces malloc);

Replaces malloc / free and provides error reporting function.

### steps

```c++
int main()
{
    int *p = new int;
    p = nullptr;
    return 0;
}
```

```bash
g++ test.cpp -fsanitize=address -fno-omit-frame-pointer
```

### options

## LeakSanitizer

### about

LeakSanitizer is integrated into AddressSanitizer.

### stand-alone mode

```bash
-fsanitize=leak
```

This will link your program against a runtime library containing just the bare necessities required for LeakSanitizer to work. No compile-time instrumentation will be applied.