#include <stdio.h>

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

int add(int a, int b)
{
    return a + b;
}

int minus(int a, int b)
{
    return a - b;
}

int main()
{
    // int a = add(1,2);
    // int b = minus(3,4);
    // return a + b;
    printf("main=%p\n", main);
    return 0;
}

