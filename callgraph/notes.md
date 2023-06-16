# callgraph

## Some tech details

### How to log funtions calls

1. Add the following codes into your program.

```c++
#ifdef __cplusplus
extern "C"
{
#endif 

void __attribute__((no_instrument_function))
__cyg_profile_func_enter (void *this_fn, void *call_site)
{
    printf("enter: %p call %p\n",
        this_fn, call_site);
}

void __attribute__((no_instrument_function))
__cyg_profile_func_exit  (void *this_fn, void *call_site)
{
    printf("leave: %p call %p\n",
        this_fn, call_site);
}

#ifdef __cplusplus
}
#endif 
```

2. Recompile modules that is used in your program and is of interest with `-finstrument-functions`.

3. Run your program and it will print function addresses.

Note: the address printed is virtual address.

### How to convert function's vir-address into function and file&lines?

1. Use getpid() or pgrep to get the pid of this process.

2. Use /proc/pid/maps or pmap to get the memory map of this process.

3. Find out the load address of the loading module(which is always the the one).

4. Calculate the offset of the address: offset = address - loadaddress.

5. Use addr2line to print function name, file, line with the offset.

### How to draw call map?

1. To be done.

Python and graphviz will be the tool to implement this.